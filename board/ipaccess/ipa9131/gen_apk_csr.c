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
#define MAX_SUBJECT_NAME_SIZE 80

#define MAX_BLOB_SIZE 304
#define MAX_PUB_KEY_PEM_BUF_SZ 1000

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
    char *mode = NULL;
    char eid[CHARACTERISATION_EID_LENGTH + 1];
    memset(subject_name,0,sizeof(subject_name));
    memset(eid,0,sizeof(eid));

    characterisation_read_eid(eid,sizeof(eid));

    if (characterisation_is_test_mode()||characterisation_is_development_mode())
    {
        mode = "Test";
    }
    else if (characterisation_is_specials_mode())
    {
        mode = "TestSpecial";
    }

    x509write_csr_init( &req );
    x509write_csr_set_md_alg( &req, POLARSSL_MD_SHA256 );

   
    snprintf(subject_name, sizeof(subject_name),SUBJECT_NAME, mode,eid);

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
    uint8_t priv_key_exp[KEY_SIZE_BYTES], black_key[268],pub_modulus[KEY_SIZE_BYTES];
    int ret = 0;
    const rsa_context *rsa = pk_rsa( *key );
    memset( priv_key_exp,0,sizeof(priv_key_exp) );
    memset( black_key,0,sizeof(black_key) );
    memset(pub_modulus,0,sizeof(pub_modulus));

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


    if ( 0 != (ret = sec_gen_priv_key_blob(priv_key_exp,sizeof(priv_key_exp), blob,black_key) ) )
    {
        goto end;
    }

    if ( 0 != mpi_write_binary( (const *) &rsa->N, (unsigned char *) pub_modulus,KEY_SIZE_BYTES) )
    {
        ret = -EFAULT;
        goto end;
    }

    sec_init_apk_ctx(pub_modulus,black_key);

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

int init_apk_black_key( const uint8_t *privkey_blob, uint32_t blob_length,const uint8_t *pubkey_data, uint32_t pub_key_len)
{
    int ret = 0;
    pk_context key;
    uint8_t pub_modulus[KEY_SIZE_BYTES];
    rsa_context *rsa;
    
    if ( !privkey_blob || !pubkey_data )
    {
        return -EINVAL;

    }

    memset(pub_modulus,0,sizeof(pub_modulus));
    pk_init( &key );


    if( 0 != (ret = pk_parse_public_key( &key ,(const unsigned char *) pubkey_data, (size_t) pub_key_len )) )
    {
        fprintf( stderr,"failed pk_parse_public_key returned 0x%08X\n", ret );
        goto end;
    }

    rsa = pk_rsa(key);

    if ( 0 != mpi_write_binary( &rsa->N, (unsigned char *) pub_modulus,KEY_SIZE_BYTES ) )
    {
	    ret = -EFAULT;
	    goto end;
    }

    if (0 != (ret = sec_init_apk_ctx_from_blob(pub_modulus,privkey_blob,blob_length)) )
    {
        fprintf( stderr,"failed sec_init_apk_ctx_from_blob returned 0x%08X\n", ret );
        goto end;

    }

end:
    pk_free(&key);
    return ret;
    

}

#if 1
/*TODO once every thing is verified, keep this inside gen_csr define*/
static int do_gen_apk_csr(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{


    int i = 0;
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
	"Excercise gen_csr to test key pair gen,priv key blob gen,csr generation",
	"gen_csr"
	);
#endif

