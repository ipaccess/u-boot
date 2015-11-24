#include "raw_container.h"
#include "hash.h"
#include "gen_apk_csr.h"
#include "flash.h"
#include "ipa9131_fuse.h"
#include "characterisation.h"
#include "sec.h"
#include <common.h>
#include <malloc.h>
#include <errno.h>
#include <fsl_sec.h>

#define IPA9131_BLOB0_START_BLOCK 25
#define IPA9131_BLOB1_START_BLOCK 26
#define IPA9131_BLOB2_START_BLOCK 1023

struct part_block_off_t
{
	uint32_t partnum;
	uint32_t  nand_block_off;
};

static const struct part_block_off_t part_num_to_block_off[]={ 
								{.partnum=3, .nand_block_off=IPA9131_BLOB0_START_BLOCK,},
								{.partnum=4, .nand_block_off=IPA9131_BLOB1_START_BLOCK,},
								{.partnum=6, .nand_block_off=IPA9131_BLOB2_START_BLOCK,},
							     };

static int hashfunc(const uint8_t * data, uint32_t data_len, uint8_t * hash_out, uint32_t hash_len)
{
	unsigned int sha256_len = SHA256_HASH_BUFFER_SIZE;
	unsigned char sha256_value[SHA256_HASH_BUFFER_SIZE];
	int ret;

	if ( 0 != (ret = hash(data,data_len,HASH_ALGO_SHA256,sha256_value,&sha256_len)) )
	{
		ret = -EFAULT;
		goto cleanup;
	}


	if (sha256_len > hash_len)
	{
		ret = -EOVERFLOW;
		goto cleanup;
	}

	if (sha256_len != hash_len)
	{
		ret = -EINVAL;
		goto cleanup;
	}

	memcpy(hash_out, sha256_value, hash_len);
	ret = 0;
cleanup:
	return ret;
	
}

static int read_raw_storage(uint32_t partnum, uint8_t ** buffer, uint32_t * buffer_length)
{
    int ret;
    int i;
    uint32_t start_addr;
    uint32_t read_size = nand_info[0].erasesize;
    unsigned char * buf = NULL;

    for (i = 0; i < (sizeof(part_num_to_block_off)/sizeof(struct part_block_off_t));i++)
    {
        if (part_num_to_block_off[i].partnum == partnum)
        {
            start_addr = flash_block_size() * flash_page_size() * part_num_to_block_off[i].nand_block_off;
            break;
        }
    }    

    if ( i >= (sizeof(part_num_to_block_off)/sizeof(struct part_block_off_t)) )
    {
        ret = -EINVAL;
        goto cleanup;
    }

    if (!(buf = calloc(read_size, sizeof(unsigned char))))
    {
        ret = -errno;
        goto cleanup;
    }


    if ( 0 != (ret = flash_read_bytes(buf,start_addr,read_size)) )
    {
	ret = -ENODATA;
        goto cleanup;
    }

    *buffer = buf;
    *buffer_length = read_size;
    buf = NULL;
    ret = 0;

cleanup:
    if (buf)
    {
        free(buf);
    }

    return ret;
}


static int read_raw_storage_container(uint32_t partnum, uint32_t * eraseblock_size, struct container_field_t ** fields, uint8_t ** data, uint32_t * len, uint8_t * faulty, uint32_t * total_length)
{
    int ret;

    *eraseblock_size = 0;
    *fields = NULL;
    *data = NULL;
    *len = 0;

    if (0 != (ret = read_raw_storage(partnum, data, len)))
    {
        *faulty = 1;
        errno = -ret;
        goto cleanup;
    }

    *eraseblock_size = *len;
    *faulty = 0;

    if (0 != (ret = parse_container(*data, *len, hashfunc, fields, total_length)))
    {
        errno = -ret;
        goto cleanup;
    }

    ret = 0;

cleanup:
    return ret;
}

static int read_raw_containers(struct raw_container_t * containers, uint32_t num_containers)
{
    uint32_t i;

    for (i = 0; i < num_containers; ++i)
    {
        read_raw_storage_container(
                containers[i].partnum,
                &containers[i].eraseblock_size,
                &containers[i].fields,
                &containers[i].data,
                &containers[i].len,
                &containers[i].faulty,
                &containers[i].length);
    }

    return 0;
}


static int create_field_in_container(struct container_field_t **fields,uint16_t tag,uint8_t *buf,uint32_t len)
{
    struct container_field_t *current;


    if (!(current = calloc(sizeof(struct container_field_t), 1)))
    {
        return -errno;
        
    }

    current->tag = tag;
    current->value = buf;
    current->length = len;

    put_container_field(fields, current);
    

    return 0;

}

static int create_container(struct container_field_t **fields)
{


    int ret = 0;
    crypt_buf_t pub_key_buf = {0}, csr_buf = {0}, priv_key_blob = {0} ;
    

    if (0 != (ret = get_key_pair_csr(&pub_key_buf,&csr_buf, &priv_key_blob)) )
    {
        fprintf(stderr,"gen_key_pair_csr returned error 0x%08X\n",ret);
	ret = -EFAULT;
        goto cleanup;

    }

    if ( ( 0 != (ret = create_field_in_container(fields,RAW_CONTAINER_TAG_PRIVATE_KEY_BLOB,priv_key_blob.buf,priv_key_blob.len)) ) ||
         ( 0 != (ret = create_field_in_container(fields,RAW_CONTAINER_TAG_PUBLIC_KEY,pub_key_buf.buf,pub_key_buf.len))    ) ||
         ( 0 != (ret = create_field_in_container(fields,RAW_CONTAINER_TAG_CSR,csr_buf.buf,csr_buf.len)) ) )
    {
        goto cleanup;
    }

    return 0;

cleanup:

    if (pub_key_buf.buf)
        free(pub_key_buf.buf);
    if (csr_buf.buf)
        free(csr_buf.buf);
    if (priv_key_blob.buf)
        free(priv_key_blob.buf);

    free_container_fields(*fields);
    *fields = NULL;
    return ret;
        

}


static int create_csr_container(struct container_field_t *pubkey_data,struct container_field_t **fields)
{   
    int ret = 0;
    crypt_buf_t csr_buf = {0};

    if (!pubkey_data)
    {
        ret = -EINVAL;
        goto cleanup;
    }


    if ( 0 != (ret = get_csr( pubkey_data->value, pubkey_data->length , &csr_buf )) )
    {
        ret = -EFAULT;
        goto cleanup;
    }

    if ( 0 != (ret = create_field_in_container(fields,RAW_CONTAINER_TAG_CSR,csr_buf.buf,csr_buf.len)) )
        goto cleanup;

    return 0;

cleanup:
    if (csr_buf.buf)
        free(csr_buf.buf);
    return ret;
}

static int create_artefact_sig_container(struct container_field_t *in_data, struct container_field_t **fields )
{
    struct container_field_t *pubkey_data = NULL , *privkey_data = NULL , *otpmk_data = NULL, *dbg_rsp_data = NULL, *csr_data = NULL;
    uint8_t *data = NULL, *p = NULL, hash[32];
    uint32_t data_len;
    int ret = -1;
    crypt_buf_t sig_buf = {0};

    memset(hash,0,sizeof(hash));

    if ( (privkey_data = find_container_field(RAW_CONTAINER_TAG_PRIVATE_KEY_BLOB, in_data)) &&
            ( pubkey_data = find_container_field(RAW_CONTAINER_TAG_PUBLIC_KEY, in_data)) &&
            ( otpmk_data = find_container_field(RAW_CONTAINER_TAG_OTPMK,in_data)) &&
            ( dbg_rsp_data = find_container_field(RAW_CONTAINER_TAG_JTAG_DBG_RSP,in_data)) && 
            ( csr_data = find_container_field(RAW_CONTAINER_TAG_CSR,in_data))
	    )
    {
        data_len = privkey_data->length + otpmk_data->length + dbg_rsp_data->length + csr_data->length;

        if (! (data = malloc(data_len)) )
        {
            ret = -ENOMEM;
            goto cleanup;

        }

        p = data;
        memcpy(p,otpmk_data->value,otpmk_data->length);
        p += otpmk_data->length;
        memcpy(p,dbg_rsp_data->value,dbg_rsp_data->length);
        p += dbg_rsp_data->length;
        memcpy(p,privkey_data->value,privkey_data->length);
        p += privkey_data->length;
        memcpy(p,csr_data->value,csr_data->length);

        if (0 != (ret = hashfunc(data,data_len,hash,32)) )
            goto cleanup;

        if ( 0 != (ret = get_signature( pubkey_data->value, pubkey_data->length , hash, 32, &sig_buf )) )
        {
            ret = -EFAULT;
            goto cleanup;
        }


        if ( 0 != (ret = create_field_in_container(fields,RAW_CONTAINER_TAG_REC_ART_SIG_DATA,sig_buf.buf,sig_buf.len)) )
            goto cleanup;    

        ret = 0;

    }

cleanup:

    if (data)
    {
        free(data);
        data = NULL;
    }

    return ret;

}

/*Validate if in container has all the required tags, if in_container is empty then create a new container with all required tags
 * else generate missing tags(if possible) data and create a new container with all the missing tags (indicate consolidation would be required in this case)*/
static int validate_and_restore_container(struct container_field_t *in_data, struct container_field_t **new_data,uint8_t *need_consolidation)
{
    int ret = 0;
    struct container_field_t *pubkey_data = NULL , *privkey_data = NULL;

    if (!in_data )
    {
	/*Allow a dev/test ap to regenarate keys if raw nand partition is empty*/
	if (characterisation_is_development_mode() || characterisation_is_test_mode())
		ret = create_container(new_data);
	else
		ret = -EFAULT;
    }
    else
    {
     
	if  ( NULL == ( privkey_data = find_container_field(RAW_CONTAINER_TAG_PRIVATE_KEY_BLOB, in_data)) )
        {
            /*No private key, return*/
            fprintf(stderr,"Private key blob not present in raw nand\n");	
            ret = -EFAULT;
	    goto cleanup;
        }


        if ( NULL == (pubkey_data = find_container_field(RAW_CONTAINER_TAG_PUBLIC_KEY, in_data)) )
        {
            /*No pub key, return*/
            fprintf(stderr,"Public key not present in raw nand\n");
            ret = -EFAULT;
            goto cleanup;

        }

        if ( 0 != sec_init_apk_from_blob(privkey_data->value, privkey_data->length) )
        {
            fprintf(stderr,"Black key intialisation failed for sec engine\n");
            ret = -EFAULT;
            goto cleanup;
        }


        if ( NULL == find_container_field(RAW_CONTAINER_TAG_CSR,in_data) )
        {
            /*No csr, generate from keys*/
            if (0 != (ret = create_csr_container(pubkey_data,new_data)) )
            {
                fprintf(stderr,"Csr generation failed with existing key pair\n");
                goto cleanup;
            }

            *need_consolidation = 1;

        }

        if ( NULL == find_container_field(RAW_CONTAINER_TAG_REC_ART_SIG_DATA,in_data) )
        {
            /*Non-fatal microloader can still continue*/
            if ( 0 != create_artefact_sig_container(in_data,new_data) ) 
                fprintf(stderr,"artefact signature generation failed with existing key pair\n");
            else
                *need_consolidation = 1;

        }
        
    }

cleanup:

    if (0 != ret)
        set_sec_state_to_fail();

    /*for Specials do not bother if validation failed or passed
     * as Specials do not use security collaterals, and can be deleted */
    if ( characterisation_is_specials_mode() )
        ret = 0;

    return ret;


}


static int update_nand_part(const struct container_field_t * fields, uint32_t partnum, uint32_t eraseblock_size)
{
    int ret, i;
    uint8_t * data;
    uint32_t len, start_addr;

    ret = 0;
    len = eraseblock_size;
    data = NULL;


    for (i = 0; i < (sizeof(part_num_to_block_off)/sizeof(struct part_block_off_t));i++)
    {
        if (part_num_to_block_off[i].partnum == partnum)
        {
            start_addr = flash_block_size() * flash_page_size() * part_num_to_block_off[i].nand_block_off;
            break;
        }
    }


    if ( i >= (sizeof(part_num_to_block_off)/sizeof(struct part_block_off_t)) )
    {
        ret = -EINVAL;
        goto cleanup;
    }

    if (!(data = calloc(sizeof(uint8_t), len)))
    {
        ret = -errno;
        goto cleanup;
    }

    if (0 != (ret = serialise_container(data, &len, hashfunc, fields)))
    {
        errno = -ret;
        fprintf(stderr,"serialise_container\n");
        goto cleanup;
    }




    nand_erase(&nand_info[0], start_addr, (size_t)eraseblock_size);

    if (0 != (ret = nand_write(&nand_info[0],start_addr,&len, data)))
    {
        fprintf(stderr,"write_buffer_to_nand failed mtd_part %d\n",partnum);
        goto cleanup;
    }
cleanup:
    if (data)
    {
        free(data);
    }

    return ret;
}

static int erase_nand_part ( uint32_t partnum, uint32_t eraseblock_size )
{
    uint32_t start_addr = 0 ,i = 0;

    for (i = 0; i < (sizeof(part_num_to_block_off)/sizeof(struct part_block_off_t));i++)
    {
        if (part_num_to_block_off[i].partnum == partnum)
        {
            start_addr = flash_block_size() * flash_page_size() * part_num_to_block_off[i].nand_block_off;
            break;
        }
    }

    if ( i >= (sizeof(part_num_to_block_off)/sizeof(struct part_block_off_t)) )
    {
        return 0;
    }

    nand_erase(&nand_info[0], start_addr, (size_t)eraseblock_size);

    return 0;


}

static int update_nand(const struct container_field_t * fields, const struct raw_container_t * nand_parts, uint32_t num_nand_parts)
{
    uint32_t i;
    uint32_t some_passed;

    some_passed = 0;

    debug_container_field(fields);

    for (i = 0; i < num_nand_parts; ++i)
    {
        if (nand_parts[i].faulty)
        {
            continue;
        }

        if (0 == update_nand_part(fields, nand_parts[i].partnum, nand_parts[i].eraseblock_size))
        {
            ++some_passed;
        }
    }

    return some_passed ? 0 : -1;
}

static int erase_nand (const struct raw_container_t * nand_parts,uint32_t num_nand_parts)
{

    int i = 0;
    for (i = 0; i < num_nand_parts; ++i)
    {

        erase_nand_part(nand_parts[i].partnum, nand_parts[i].eraseblock_size);
        
    }

    return 0;
}

int do_restore_container(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{

    int ret = 0;
    struct raw_container_t raw_containers[10];
    uint32_t num_raw_containers;
    uint32_t i = 0;
    struct container_field_t * new_data;
    struct container_field_t * nand_data;
    struct container_field_t * consolidated_data;
    uint8_t need_consolidation; 
    unsigned long ulval;
    char * ep;

    nand_data = NULL;
    new_data=NULL;
    consolidated_data= NULL;
    memset(raw_containers, 0, sizeof(raw_containers));
    num_raw_containers = 0;
    need_consolidation = 0;

    while(i < (argc - 1))
    {
        if (0 == strcmp(argv[i],"-m"))
        {
            ulval = simple_strtoul(argv[i+1], &ep, 10);

            if (ep == argv[i+1] || *ep != '\0')
            {
                fprintf(stderr, "Invalid mtd part num, not a decimal value\n");

                return CMD_RET_FAILURE;
            }

            raw_containers[num_raw_containers].partnum = (uint32_t)(ulval & 0xFFFFFFFF);
            ++num_raw_containers;
        }
        ++i;
    }

    if (0 != (ret = read_raw_containers(raw_containers, num_raw_containers)))
    {
        errno = -ret;
        ret = 1;
        fprintf(stderr,"read_raw_containers\n");
        goto cleanup;
    }


    for (i = 0; i < num_raw_containers; ++i)
    {

        if (raw_containers[i].fields)
        {
            nand_data = raw_containers[i].fields;
        }
        else
        {
            if (i != (num_raw_containers-1) )
                continue;

        }


        if ( (0 == ( ret = validate_and_restore_container(nand_data,&new_data,&need_consolidation)) ) )
        {
            if (new_data)
            {    
                if (need_consolidation)
                    container_union(new_data,nand_data,&consolidated_data);
                else
                    consolidated_data = new_data;

                if (0 != (ret = update_nand(consolidated_data,raw_containers, num_raw_containers)))
                {
                    fprintf(stderr, "Failed to update any NAND partitions\n");
                }
            }
            break;

        }
        else
        {
            fprintf(stderr,"validation and restoration of container failed mtd_part %d\n",raw_containers[i].partnum);
            if ( i == (num_raw_containers -1))
                break;
            else
                continue;
        }

    }

cleanup:

    for (i = 0; i < num_raw_containers; ++i)
    {
        free_container_fields(raw_containers[i].fields);

        if (raw_containers[i].data)
        {
            free(raw_containers[i].data);
        }
    }

    if (new_data != consolidated_data)
	    free_container_fields(consolidated_data);

    free_container_fields_and_values(new_data);
    if ( 0 == ret )
    {
        return CMD_RET_SUCCESS;
    }
    else
    {
        cli_simple_run_command("ledc nwk yellow", 0);
        udelay(10000000);
        cli_simple_run_command("ledc nwk off yellow 3 500", 0);
        return CMD_RET_FAILURE;
    }

}


static int toughen_otpmk(struct raw_container_t *raw_containers,uint32_t num_raw_containers)
{
    struct container_field_t * otpmk_data = NULL;
    crypt_buf_t otpmk_buf = {0};
    uint32_t otpmk[8];
    uint32_t otpmk_fused = 0x01;
    int ret = 1,i = 0;
    uint32_t secmon_hpsr = 0;

    memset(otpmk,0,sizeof(otpmk));

    if (0 != read_raw_containers(raw_containers, num_raw_containers))
    {
        fprintf(stderr,"toughen_otpmk:read_raw_containers\n");
        goto cleanup;
    }

    /*For the case where previous provisioning failed to set the SFP_OSCR bit to indicate that otpmk is fused
     *in such a case Abort the provisioning in order not to write the otpmk fuses again which might brick the board*/
    for (i = 0; i < num_raw_containers ; i++)
    {
        if(raw_containers[i].fields && find_container_field(RAW_CONTAINER_TAG_OTPMK,raw_containers[i].fields))
        {

            fprintf(stderr,"toughen_otpmk:otpmk already exist in nand, Abort\n");
            goto cleanup;           

        }
    }

    if ( 0 != get_otpmk(otpmk,&otpmk_buf) )
    {
        fprintf(stderr,"toughen_otpmk:get_otpmk failed\n");
        goto cleanup;
    }


    if ( 0 != create_field_in_container(&otpmk_data,RAW_CONTAINER_TAG_OTPMK,otpmk_buf.buf,otpmk_buf.len) ) 
        goto cleanup;

    otpmk_buf.buf = NULL;

    ipa9131_fuse_init();

    if (0 != ipa9131_fuse_write_in_range(SFP_OTPMKR0_ADDRESS,8,otpmk)) 
    {
        fprintf(stderr,"toughen_otpmk:setting otpmk fuses failed\n");
        goto cleanup;

    }

    udelay(10000);
    secmon_hpsr = sec_in_be32(SECMON_HPSR);

    /*Sec mon hpsr almost immediately reflects error if OTPMK is not hamming protected*/
    if (secmon_hpsr & 0x09FF0000)
    {
        fprintf(stderr,"toughen_otpmk:Wrong value in OTPMK registers, can't blow fuses %08X\n",secmon_hpsr);
        goto cleanup;
    }

    if (0 != ipa9131_fuse_write_in_range(SFP_OSCR_ADDRESS,1,&otpmk_fused))
    {
        fprintf(stderr,"toughen_otpmk:setting otpmk oscr bit failed\n");
        goto cleanup;

    }


    if (0 != update_nand(otpmk_data,raw_containers, num_raw_containers))
    {
        fprintf(stderr, "toughen_otpmk:Failed to update any NAND partitions\n");
        goto cleanup;
    }

    /*Finally write the fuses for otpmk*/
    ipa9131_blow_fuse(); 
    ret = 0;

cleanup:
    if (0 != ret)
        set_sec_state_to_fail();

    for (i = 0; i < num_raw_containers; ++i)
    {
        free_container_fields(raw_containers[i].fields);
        raw_containers[i].fields = NULL;

        if (raw_containers[i].data)
        {
            free(raw_containers[i].data);
            raw_containers[i].data = NULL;
        }
    }

    free_container_fields_and_values(otpmk_data);
    if (otpmk_buf.buf)
        free(otpmk_buf.buf);

    return ret;


}

static int toughen_dbg_rsp(struct raw_container_t *raw_containers,uint32_t num_raw_containers)
{
    struct container_field_t * dbg_rsp_data = NULL, * nand_data = NULL, *consolidated_data = NULL;
    crypt_buf_t dbg_rsp_buf = {0};
    uint32_t dbg_rsp[2];
    uint32_t dbg_rsp_val = 0x02;
    int ret = 1, i = 0;

    memset(dbg_rsp,0,sizeof(dbg_rsp));

    if (0 != read_raw_containers(raw_containers, num_raw_containers))
    {
        fprintf(stderr,"toughen_dbg_rsp:read_raw_containers\n");
        goto cleanup;
    }

    /*For the case where previous provisioning failed to set the SFP_OSCR bit to indicate that dbg rsp is fused
     * in such a case Abort the provisioning in order not to write the dbg rsp fuses again which might brick the board*/
    for (i= 0; i < num_raw_containers ; i++)
    {
        if (raw_containers[i].fields && find_container_field(RAW_CONTAINER_TAG_JTAG_DBG_RSP,raw_containers[i].fields))
        {

            fprintf(stderr,"toughen_dbg_rsp: dbg rsp already exist in nand, Abort\n");
            goto cleanup;

        }
    }

    for (i = 0; i < num_raw_containers; ++i)
    {

        if (raw_containers[i].fields)
        {
            nand_data = raw_containers[i].fields;
            break;
        }
    }

    if ( 0 != get_dbg_rsp(dbg_rsp,&dbg_rsp_buf) )
    {
        fprintf(stderr,"toughen_dbg_rsp:get_dbg_rsp failed\n");
        goto cleanup;
    }

    if ( 0 != create_field_in_container(&dbg_rsp_data,RAW_CONTAINER_TAG_JTAG_DBG_RSP,dbg_rsp_buf.buf,dbg_rsp_buf.len) ) 
        goto cleanup;

    dbg_rsp_buf.buf = NULL;

    if (nand_data)
        container_union(dbg_rsp_data,nand_data,&consolidated_data);
    else
        consolidated_data = dbg_rsp_data;


    ipa9131_fuse_init();

    if (0 != ipa9131_fuse_write_in_range(SFP_DRVR0_ADDRESS,2,dbg_rsp) )
    {
        fprintf(stderr,"toughen_dbg_rsp:setting dbg_rsp fuses failed\n");
        goto cleanup;

    }


    if (0 != ipa9131_fuse_write_in_range(SFP_OSCR_ADDRESS,1,&dbg_rsp_val) )
    {
        fprintf(stderr,"toughen_otpmk:setting otpmk oscr bit failed\n");
        goto cleanup;

    }


    if (0 != update_nand(consolidated_data,raw_containers, num_raw_containers))
    {
        fprintf(stderr, "Failed to update any NAND partitions\n");
        goto cleanup;
    }

    /*Finally write the fuses for dbg_rsp*/
    ipa9131_blow_fuse();

    ret = 0;
cleanup:
    if (0 != ret)
	    set_sec_state_to_fail();
    for (i = 0; i < num_raw_containers; ++i)
    {
        free_container_fields(raw_containers[i].fields);
        raw_containers[i].fields = NULL;

        if (raw_containers[i].data)
        {
            free(raw_containers[i].data);
            raw_containers[i].data = NULL;
        }
    }

    if (consolidated_data != dbg_rsp_data) 
        free_container_fields(consolidated_data);

    free_container_fields_and_values(dbg_rsp_data);
    if (dbg_rsp_buf.buf)
        free(dbg_rsp_buf.buf);
    return ret;

}

static int gen_apk_container(struct raw_container_t *raw_containers,uint32_t num_raw_containers)
{
    struct container_field_t * apk_data = NULL, **nand_data = NULL, *consolidated_data = NULL, *field = NULL;
    uint16_t tag_list[] = {RAW_CONTAINER_TAG_PRIVATE_KEY_BLOB,RAW_CONTAINER_TAG_PUBLIC_KEY,RAW_CONTAINER_TAG_CSR,0};
    uint32_t apk_created_val = 0x04,write_protect = 0x01;
    int ret = 1, i = 0;

    if (0 != read_raw_containers(raw_containers, num_raw_containers))
    {
        fprintf(stderr,"gen_apk_container:read_raw_containers failed\n");
        goto cleanup;
    }

    for (i = 0; i < num_raw_containers; ++i)
    {

        if (raw_containers[i].fields)
        {
            nand_data = &raw_containers[i].fields;
            break;
        }
    }

    /*Remove private key blob,public key,csr if already present, they might be present if previous provisioning attempt failed
     * to set the SFP_OSCR apk fuse bit, discard the older data generate afresh*/
    if (*nand_data)
    {
        for (i = 0; 0 != tag_list[i]; ++i)
        {
            field = find_container_field(tag_list[i], *nand_data);

            if (field)
            {
                take_container_field(nand_data, field);
                free_container_fields(field);
                field = NULL;
            }
        }

    }

    if (0 != create_container(&apk_data))
    {
        fprintf(stderr,"gen_apk_container:failed creating apk_container\n");
        goto cleanup;
    }

    if (*nand_data)
        container_union(apk_data,*nand_data,&consolidated_data);
    else
        consolidated_data = apk_data;

    ipa9131_fuse_init();

    if (0 != ipa9131_fuse_write_in_range(SFP_OSCR_ADDRESS,1,&apk_created_val) )
    {
        fprintf(stderr,"gen_apk_container:setting apk oscr bit failed\n");
        goto cleanup;

    }

    if (0 != ipa9131_fuse_write_in_range(SFP_OSPR_ADDRESS,1,&write_protect) )
    {
        fprintf(stderr,"gen_apk_container:setting write_protect bit in fuses failed\n");
        goto cleanup;

    }


    if (0 != update_nand(consolidated_data,raw_containers, num_raw_containers))
    {
        fprintf(stderr, "gen_apk_container:Failed to update any NAND partitions\n");
        goto cleanup;
    }

    /*Everything went as expected, Last provisioning bit comming to end, No More fuse blowing
     * possible after this*/
    ipa9131_blow_fuse();

    ret = 0;

cleanup:

    if (0 != ret)
	    set_sec_state_to_fail();

    for (i = 0; i < num_raw_containers; ++i)
    {
        free_container_fields(raw_containers[i].fields);
        raw_containers[i].fields = NULL;

        if (raw_containers[i].data)
        {
            free(raw_containers[i].data);
            raw_containers[i].data = NULL;
        }
    }

    if (consolidated_data != apk_data)
        free_container_fields(consolidated_data);

    free_container_fields_and_values(apk_data);
    return ret;


}

int do_provisioning(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{

    int ret = 0;
    struct raw_container_t raw_containers[10];
    uint32_t num_raw_containers;
    uint32_t i = 0;
    unsigned long ulval;
    char * ep;
    u8 otpmk_set = 0, dbg_resp_set = 0, apk_created = 0;

    memset(raw_containers, 0, sizeof(raw_containers));
    num_raw_containers = 0;

    ipa9131_fuse_init();
    /*always re-read the fuses in order to read the actual fused values, not what might be set in shadow registers*/
    ipa9131_read_provisioning_status(&otpmk_set,&dbg_resp_set,&apk_created);


    if ( (otpmk_set && dbg_resp_set && apk_created) )
        return CMD_RET_SUCCESS;

    /*safeguard, provisioning to be run only in case of secure boot*/
    if ( !ipa9131_fuse_its_blown() )
    {
        fprintf(stderr,"Secure boot not enabled!! Failed to run provisioning\n");
        return CMD_RET_FAILURE;
    }


    while(i < argc)
    {
        if (0 == strcmp(argv[i],"-m"))
        {
            ulval = simple_strtoul(argv[i+1], &ep, 10);

            if (ep == argv[i+1] || *ep != '\0')
            {
                fprintf(stderr, "Invalid mtd part num, not a decimal value\n");

                return CMD_RET_FAILURE;
            }

            raw_containers[num_raw_containers].partnum = (uint32_t)(ulval & 0xFFFFFFFF);
            ++num_raw_containers;
        }
        ++i;
    }

    if (!otpmk_set)
    {
        /*Should happen only once in Ap lifetime*/
        if (0 != toughen_otpmk(raw_containers,num_raw_containers))
        {
            cli_simple_run_command("ledc nwk yellow", 0);
            udelay(10000000);
            cli_simple_run_command("ledc nwk off yellow 1 500", 0);
        }

        return CMD_RET_FAILURE;
        /*irrespective of whether toughen_otpmk returned true or false
         *This will always return false, as the AP must reboot after this 
         */
    }

    if (!dbg_resp_set)
    {	/*Should happen only once in Ap lifetime*/
        if ( 0 != toughen_dbg_rsp(raw_containers,num_raw_containers) )
        {
            cli_simple_run_command("ledc nwk yellow", 0);
            udelay(10000000);
            cli_simple_run_command("ledc nwk off yellow 2 500", 0);
            return CMD_RET_FAILURE;
        }
    }

    if (!apk_created)
    {
        /*Should happen only once in Ap lifetime*/
        if (0 != gen_apk_container(raw_containers,num_raw_containers) )
        {
            cli_simple_run_command("ledc nwk yellow", 0);
            udelay(10000000);
            cli_simple_run_command("ledc nwk off yellow 3 500", 0);
            return CMD_RET_FAILURE;
        }
    }

    return CMD_RET_SUCCESS;

}


#if defined ML9131_PROVISIONING_COMMANDS
U_BOOT_CMD(restore_raw_container, 9, 0, do_restore_container,
		"Exercise restore_raw_container",
		"-m <mtd-part-num>"
		"up to 4 mtd part num can be provided"
	  );

U_BOOT_CMD(ipa9131_provisioning, 9, 0, do_provisioning,
        "Exercise initial_provisioning",
        "ipa9131_provisioning -m <mtd-part-num>"
        "up to 4 mtd part num can be provided"
        );
#endif
