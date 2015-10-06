#include <common.h>
#include <polarssl/config.h>
#include <polarssl/rsa.h>
#include <polarssl/x509_csr.h>
#include <fsl_sec.h>
#include <polarssl/cipher.h>
#include <polarssl/pk.h>
#include <errno.h>
#include <malloc.h>

#include "gen_apk_csr.h"
#include "characterisation.h"
#include "ipa9131_fuse.h"

#define KEY_SIZE_BITS 2048
#define KEY_SIZE_BYTES 256
#define EXPONENT 65537

#define ADVERT_NUM_OCTETS 256

#define CSR_MAX_BUFF_SIZE 4096

#define SUBJECT_NAME "C=GB,L=%s,O=ip.access Ltd,OU=3GAP,CN=%s"
#define SUBJECT_NAME_PROD "C=GB,O=ip.access Ltd,OU=3GAP,CN=%s"
#define MAX_SUBJECT_NAME_SIZE 80

#define MAX_BLOB_SIZE 304
#define MAX_PUB_KEY_PEM_BUF_SZ 1000

u8 oem_ca_pub_modulus[256] = { 0xbb,0x3b,0x97,0x70,0x3e,0x1f,0x51,0x80,0x7e,0xff,0x45,0x25,0xe4,0xe7,0x45,0xe7,0xac,0x42,0x45,0x36,0x26,0x78,0x1e,0x6b,0xc6,0xca,0x02,0x64,0x5d,
			       0x1c,0xb9,0xb4,0x31,0x00,0x3c,0x24,0x97,0xb0,0x32,0x8e,0x67,0x81,0xa4,0x79,0x48,0x74,0x37,0x80,0xa2,0x71,0x05,0x74,0x94,0xaa,0x16,0x80,0x66,0xbc,
			       0x76,0x5d,0xdd,0x61,0x61,0xfe,0x63,0xf8,0x32,0x20,0x84,0x41,0x17,0x72,0xb1,0x3e,0x79,0x29,0xa1,0xd9,0x4e,0x2d,0xe6,0x8e,0x0f,0x7b,0x48,0x39,0xf5,
			       0xc9,0x76,0x11,0x50,0x7c,0x14,0x4d,0x5d,0x49,0x3d,0xef,0x00,0xf9,0x3a,0xd4,0x0f,0xa1,0xff,0xa6,0x12,0xdb,0x2a,0x5e,0xd9,0x70,0x2d,0x99,0x37,0x4a,
			       0x44,0xcf,0x07,0xb8,0x11,0xb7,0x37,0x47,0xd5,0x67,0xb6,0x82,0x77,0x80,0x24,0x2a,0xbc,0x59,0x8e,0x23,0xe3,0x02,0xba,0x9b,0x4e,0x01,0x00,0xc6,0xae,
			       0x79,0x6b,0x87,0x0e,0xb7,0x4e,0x20,0x6c,0x53,0x93,0xa1,0x5d,0xfa,0xdd,0x86,0x10,0xd2,0xb4,0x0c,0x4b,0x03,0x57,0x6b,0xc9,0x08,0x41,0xa8,0xbf,0x10,
			       0xb5,0x6d,0xf1,0xb9,0x4b,0xd4,0xd4,0x30,0x86,0xb8,0xc3,0x44,0x6e,0x53,0xbf,0xd1,0xe4,0x55,0x9f,0x16,0x3d,0x53,0x90,0x3b,0x1e,0x98,0x37,0xb2,0x45,
			       0xdb,0x5e,0x97,0x11,0xf1,0x73,0xc6,0x85,0xaa,0x95,0xb7,0x26,0x31,0xbe,0xa4,0xe1,0x74,0x37,0x6f,0x67,0xe3,0xc4,0x95,0x98,0x9b,0x28,0xc7,0x49,0x75,
			       0x3f,0xac,0x1e,0xec,0x37,0x45,0x0a,0x69,0x98,0xcc,0x6c,0x76,0x42,0xd6,0x4c,0xea,0xcd,0x7a,0x3c,0xa5,0x58,0xab,0x57,0x97};

int oem_ca_pub_exponent = 50457;


static int gen_rand_num (void *prng, unsigned char * rand_num, size_t length)
{
    int ret=0;
    ret = sec_generate_random_number(length,(uint8_t *)rand_num);
    return ret;
}

static int gen_rsa_key( pk_context *key )
{
    int ret;

    rsa_init( pk_rsa( *key ), RSA_PKCS_V15, 0 );
    if( ( ret = rsa_gen_key( pk_rsa( *key ), gen_rand_num, NULL, KEY_SIZE_BITS,
                    EXPONENT ) ) != 0 )
    {
        fprintf( stderr,"failed rsa_gen_key returned 0x%04X\n", ret );
        goto end;
    }

end:
    return ret;
}


static int write_certificate_request( x509write_csr *req, crypt_buf_t *csr_out_buf )
{
    int ret;
    unsigned char *out_buf =  NULL;
    int len = 0;

    if ( !(out_buf = (unsigned char *)calloc(CSR_MAX_BUFF_SIZE, sizeof(unsigned char))) )
    {
        ret = -errno;
        goto end;
    }

    if( 0 != ( ret = x509write_csr_pem( req, out_buf, CSR_MAX_BUFF_SIZE, gen_rand_num, NULL ) )  )
    {
        goto end;
    }


    if ( (len = strlen(out_buf)) < 1 )
    {
	ret = -EFAULT;
	goto end;
    }

    csr_out_buf->buf = out_buf;
    csr_out_buf->len = len;
    out_buf=NULL;

end:
    if (out_buf)
        free(out_buf);
    return ret;

}


static int gen_csr_pem( pk_context *key,crypt_buf_t *csr_out_buf )
{
    int ret = 0;
    x509write_csr req;
    char subject_name[MAX_SUBJECT_NAME_SIZE];
    char eid[CHARACTERISATION_EID_LENGTH + 1];
    memset(subject_name,0,sizeof(subject_name));
    memset(eid,0,sizeof(eid));

    characterisation_read_eid(eid,sizeof(eid));


    x509write_csr_init( &req );
    x509write_csr_set_md_alg( &req, POLARSSL_MD_SHA256 );

   
    if (characterisation_is_test_mode()||characterisation_is_development_mode())
	    snprintf(subject_name, sizeof(subject_name),SUBJECT_NAME,"Test",eid);
    else if (characterisation_is_specials_mode())
	    snprintf(subject_name, sizeof(subject_name),SUBJECT_NAME,"TestSpecial",eid);
    else
	    snprintf(subject_name, sizeof(subject_name),SUBJECT_NAME_PROD,eid);

    if( ( ret = x509write_csr_set_subject_name( &req, subject_name ) ) != 0 )
    {
        fprintf(stderr," failed x509write_csr_set_subject_name returned %8X", ret );
        goto end;
    }

    /*Set the sec engine function for doing private key operations*/
    rsa_set_ext_private_func(pk_rsa( *key ), sec_do_rsa_private);

    x509write_csr_set_key( &req, key );

    if( ( ret = write_certificate_request( &req,csr_out_buf) ) != 0 )
    {
        fprintf(stderr," failed write_certifcate_request %8X", ret );
        goto end;
    }

end:
    x509write_csr_free( &req );
    return ret;
}


static int get_pub_key_buf( pk_context *key, crypt_buf_t *out_buf )
{
    int ret = 0;
    unsigned char *pub_key_buf=NULL;
    uint32_t len = 0;

    if ( !(pub_key_buf = (unsigned char *)malloc(MAX_PUB_KEY_PEM_BUF_SZ)) )
    {
        ret = -errno;
        goto end;
    }

    memset(pub_key_buf, 0,MAX_PUB_KEY_PEM_BUF_SZ);

    if( ( ret = pk_write_pubkey_pem( key, pub_key_buf, MAX_PUB_KEY_PEM_BUF_SZ ) ) != 0 )
        goto end;

    if ( !(len = strlen((char *)pub_key_buf)) )
    {
	    ret = -EFAULT;
	    goto end;
    }

    out_buf->buf = pub_key_buf;
    out_buf->len = len;
    pub_key_buf = NULL;
    
end:
    if(pub_key_buf)
        free(pub_key_buf);
    return ret;

}


int get_priv_key_blob( pk_context *key, crypt_buf_t *priv_key_blob )
{

    uint8_t *blob=NULL;
    uint8_t priv_key_exp[KEY_SIZE_BYTES];
    int ret = 0;
    const rsa_context *rsa = pk_rsa( *key );
    memset( priv_key_exp,0,sizeof(priv_key_exp) );

    if ( 0 != mpi_write_binary( &rsa->D, (unsigned char *) priv_key_exp,KEY_SIZE_BYTES) )
    {
        ret = -EFAULT;
        goto end;
    }


    if ( !(blob = (uint8_t *)malloc(MAX_BLOB_SIZE)))
    {
        ret = -errno;
        goto end;
    }


    if ( 0 != (ret = sec_gen_priv_key_blob(priv_key_exp,sizeof(priv_key_exp), blob) ) )
    {
        goto end;
    }
    
    priv_key_blob->buf = blob;
    priv_key_blob->len = MAX_BLOB_SIZE;
    blob = NULL;

end:
    if (blob)
        free(blob);
    return ret;

}

int get_key_pair_csr(crypt_buf_t *pub_key_buf, crypt_buf_t *csr_buf,crypt_buf_t *priv_key_blob )
{
    int ret = 0;
    pk_context key;

    pk_init( &key );
    if( ( ret = pk_init_ctx( &key, pk_info_from_type(POLARSSL_PK_RSA)) ) != 0 )
    {
        fprintf( stderr,"failed pk_init_ctx returned 0x%04X", ret );
        goto end;
    }

    if( (ret = gen_rsa_key(&key) ) != 0 )
    {
        fprintf(stderr,"failed generating Key pair 0x%08X\n",ret);
        goto end;
    }

    if (0 != (ret = get_pub_key_buf(&key,pub_key_buf)) )
    {
	fprintf(stderr,"failed getting pub key in pem 0x%08X\n",ret);
        goto end;
    }

    if (0 != (ret = get_priv_key_blob(&key,priv_key_blob)))
    {
	    fprintf(stderr,"failed getting priv Key blob 0x%08X\n",ret);
	    goto end;
    }

    if ( 0 != (ret = gen_csr_pem(&key,csr_buf)) )
    {
        fprintf( stderr,"failed generating CSR 0x%08X \n",ret);
        goto end;

    }
end:
    pk_free(&key);
    return ret;

}


int get_csr( const uint8_t *pub_key_buf, uint32_t pub_key_len , crypt_buf_t *csr_buf )
{
    int ret = 0;
    pk_context key;

    pk_init( &key );


    if( 0 != (ret = pk_parse_public_key( &key ,(const unsigned char *) pub_key_buf, (size_t) pub_key_len )) )
    {
        fprintf( stderr,"failed pk_parse_public_key returned 0x%08X\n", ret );
        goto end;
    }

    if ( 0 != (ret = gen_csr_pem(&key,csr_buf)) )
    {
        fprintf( stderr,"failed gen_csr_pem returned 0x%08X\n",ret);
        goto end;

    }

end:
    pk_free(&key);
    return ret;


}

static int generate_hamming_code(const u8 *in_buf,u32 len_bits,u8 *ham_code)
{

    u32 len_bytes,i = 0,j = 0;
    u8 *num;
    u8 temp = 0,l = 0;

    len_bytes = len_bits/8;

    if ( !(num = (u8 *)calloc(len_bits,sizeof(u8))) )
    {
	  return -ENOMEM;
	   
    }

    for (i = 0; i < len_bytes; i++) 
    {

        l = 0x80;
        for (j = 0; j < 8; j++) 
        {
            num[i * 8 + j] = !!(in_buf[i] & l);
            l = l >> 1;
        }
    }


    /* Calculate each code bit in turn */
    for (i = 1; i <= (len_bits / 2); i = (i << 1)) 
    {
        /* Examine each data bit
         * Only bits greater than i need to be checked as no
         * bit less than i will ever be XORed into i
         * J starts at i so that number[i] is initialized to 0
         * */
        for (j = i; j <= (len_bits - 1); j++) 
        {
            if ((i & j) != 0)
                num[i] = num[i] ^ num[j];
        }
    }
    /* Calculate the overall parity
     * J starts at 0 so that number[0] is initialized to 0
     * number[0] contains the even parity of all of the bits
     * */
    for (j = 0; j <= len_bits - 1; j = j + 1)
        num[0] = num[0] ^ num[j];

    for (i=0;i < len_bytes;i++)
    {
        temp = 0;
        for (j=0;j < 8; j++)
        {
            temp = (temp << 1)| num[ i*8 + j];
        }

        ham_code[i]= temp;

    }

    free(num);
    return 0;


}

static int generate_hamming_code_B(const u8 *in_buf,u32 len_bits,u8 *ham_code)
{

    u32 len_bytes,i = 0,j = 0;
    u8 *num;
    u8 parity = 0,l = 0;

    len_bytes = len_bits/8;

    if ( !(num = (u8 *)calloc(len_bits,sizeof(u8))) )
    {
        return -ENOMEM;

    }

    /*populate bit array with reversed bytes from input buffer
     * weird but that's what freescale does, not sure why*/
    for (i = 0; i < len_bytes; i++) 
    {

        l = 0x80;
        for (j = 0; j < len_bytes; j++) {
            num[len_bytes * 8 - i * 8 - 1 - j] = !!(in_buf[i] & l);
            l = l >> 1;
        }
    }

    /* response must be masked out the hamming coding bits first */
    for (i = 0; i < len_bits; i = (2*(i+1) - 1))
        num[i] = 0;

    /* Calculate each code bit in turn */
    for (i = 0; i < (len_bits/2); i = (2*(i+1) - 1)) 
    {
        parity = num[i];

        for (j = i+1; j < len_bits; j++) 
        {
            if (((i+1) & (j+1)) != 0)
                parity ^= num[j];

            num[i] = (num[i] & 0) | parity;
        }
    }

    /* Calculate the overall parity */
    parity = 0;
    for (j = 0; j < len_bits; j++)
        parity ^= num[j];

    num[63] = num[63] | parity;

    memset(ham_code,0,len_bytes);

    /*Populate back the hammming code, again reading from the end first*/
    for (i = 0; i < len_bytes; i++) 
    {
        l = 7;
        for (j = 0; j < 8; j++) {
            ham_code[i] |= num[(len_bytes - i) * 8 - (j + 1)] << l;
            --l;
        }
    }


    free(num);
    return 0;
}




static int oem_rsa_encrypt(uint8_t *in_buf,uint32_t in_len,uint8_t *out_buf,uint32_t *out_len )
{
	int ret = 0;
	rsa_context rsa;

	rsa_init( &rsa, RSA_PKCS_V15, 0 );

	if ( (ret = mpi_read_binary( &rsa.N, oem_ca_pub_modulus, sizeof(oem_ca_pub_modulus) )) != 0 )
	{
		fprintf( stderr,"oem_rsa_encrypt:failed creating rsa context -0x%04X", -ret );
		goto end;

	}

	if ( (ret = mpi_lset(&rsa.E,oem_ca_pub_exponent)) != 0 )
	{
		fprintf( stderr,"oem_rsa_encrypt:failed creating pub_exp rsa xontext -0x%04X", -ret );
		goto end;
	}

	rsa.len = ( mpi_msb( &rsa.N ) + 7 ) >> 3;


	if( ( ret = rsa_pkcs1_encrypt( &rsa, gen_rand_num, NULL,
					RSA_PUBLIC, in_len,
					in_buf, out_buf ) ) != 0 )
	{
		fprintf( stderr," failed rsa_pkcs1_encrypt returned -0x%04X\n", -ret );
		goto end;
	}

	*out_len = rsa.len;

end:
	return ret;

		

}

int get_otpmk(u32 *otpmk,crypt_buf_t *otpmk_buf)
{
    u8 rand_buf[32],ham_code[32];
    u32 len_bytes = 32, len_bits = 256,enc_len = 256;
    u8 encrypted_buf[256],otpmk_pt_buf[64];
    int ret = 0;
    u32 *p,i;

    if ( !otpmk || !otpmk_buf)
        return -EINVAL;    

    memset(encrypted_buf,0,sizeof(encrypted_buf));
    memset(rand_buf,0,sizeof(rand_buf));
    memset(ham_code,0,sizeof(ham_code));
    memset(otpmk_pt_buf,0,sizeof(otpmk_pt_buf));

    if (0 != (ret = sec_generate_random_number(len_bytes,rand_buf)) )
	    goto cleanup;


    rand_buf[31] = rand_buf[31] | IPA9131_MINIMAL_OTPMK_VALUE;

    if (0 != (ret = generate_hamming_code(rand_buf,len_bits,ham_code)) )
        goto cleanup;


    p = (u32 *) &ham_code[len_bytes];
    --p;
    for (i =0;i < len_bytes/4;i++)
    {
        otpmk[i] = *p;
        --p;
    }


    for ( i=0;i < len_bytes;i++ )
	snprintf((char *) &otpmk_pt_buf[i*2],2*sizeof(char),"%02X",ham_code[i]);


    if (0 != (ret = oem_rsa_encrypt(otpmk_pt_buf,sizeof(otpmk_pt_buf),encrypted_buf,&enc_len )) )
	goto cleanup;

    if (enc_len > 256)
    {
	    ret = -EFAULT;
	    goto cleanup;
    }

    if ( !(otpmk_buf->buf = (u8 *)calloc(enc_len,sizeof(u8))) )
    {
	    ret = -ENOMEM;
            goto cleanup;

    }

    memcpy(otpmk_buf->buf,encrypted_buf,enc_len);
    otpmk_buf->len = enc_len;

cleanup:
    return ret;


}

int get_dbg_rsp(u32 *dbg_rsp,crypt_buf_t *dbg_rsp_buf)
{
    u8 rand_buf[8] = {0x69,0x70,0x61,0x63,0x63,0x65,0x73,0x73};
    /*Default value to be used for Development/Test Aps*/ 
    u8 ham_code[8];
    u32 len_bytes = 8, len_bits = 64;
    u8 encrypted_buf[256], dbg_pt_buf[16];
    u32 enc_len = 256;
    u32 *p,i;	
    int ret = 0;
    if (!dbg_rsp || !dbg_rsp_buf)
	    return -EINVAL;

    memset(dbg_rsp,0,len_bytes);
    memset(ham_code,0,sizeof(ham_code));
    memset(encrypted_buf,0,sizeof(encrypted_buf));
    memset(dbg_pt_buf,0,sizeof(dbg_pt_buf));

    if ( characterisation_is_production_mode() || characterisation_is_specials_mode() ) 
    {
    	memset(rand_buf,0,sizeof(rand_buf));
        /*Only production/specials have random debug value*/
        if (0 != (ret = sec_generate_random_number(len_bytes,rand_buf)) )
            goto cleanup;
    }


    rand_buf[0] |= (IPA9131_MINIMAL_JTAG_RESP_VALUE >> 24) & 0xFF;
    
    

    if (0 != (ret = generate_hamming_code_B(rand_buf,len_bits,ham_code)) )
        goto cleanup;


    p = (u32 *) &ham_code[len_bytes];
    --p;
    for (i =0;i < len_bytes/4;i++)
    {
        dbg_rsp[i] = *p;
        --p;
    }


    for ( i=0;i < len_bytes;i++ )
	    snprintf((char *) &dbg_pt_buf[i*2],2*sizeof(char),"%02X",ham_code[i]);

    if (0 != (ret = oem_rsa_encrypt(dbg_pt_buf,sizeof(dbg_pt_buf),encrypted_buf,&enc_len )) )
	    goto cleanup;

    if (enc_len > 256)
    {
	    ret = -EFAULT;
	    goto cleanup;
    }

    if ( !(dbg_rsp_buf->buf = (u8 *)calloc(enc_len,sizeof(u8))) )
    {
	    ret = -ENOMEM;
            goto cleanup;

    }

    memcpy(dbg_rsp_buf->buf,encrypted_buf,enc_len);
    dbg_rsp_buf->len = enc_len;

cleanup:
    return ret;


}

#if defined CONFIG_CMD_GEN_APK_CSR
static int do_gen_apk_csr(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{


    crypt_buf_t pubkey = {0}, csr = {0} , priv_key_blob = {0};

    if ( !get_key_pair_csr(&pubkey,&csr,&priv_key_blob ) && pubkey.buf && csr.buf && priv_key_blob.buf)
    {
	printf("**Generated pubkey** Length %d \n %s\n",pubkey.len,pubkey.buf);
	printf("\n**Generated csr** Length %d \n %s\n",csr.len,csr.buf);

	free(pubkey.buf);
        free(csr.buf);
	free(priv_key_blob.buf);

        return CMD_RET_SUCCESS;

    }
    else
    {
        if (pubkey.buf)
            free(pubkey.buf);
        if(csr.buf)
            free(csr.buf);
	if (priv_key_blob.buf)
            free(priv_key_blob.buf);

        return CMD_RET_FAILURE;
    }


}

U_BOOT_CMD(gen_apk_csr, 2, 0, do_gen_apk_csr,
	"Test Key pair and Csr generation ",
	"gen_apk_csr"
	);
#endif

