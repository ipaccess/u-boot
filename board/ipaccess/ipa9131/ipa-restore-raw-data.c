#include "raw_container.h"
#include "hash.h"
#include "gen_apk_csr.h"
#include "flash.h"
#include <common.h>
#include <malloc.h>
#include <errno.h>

#define IPA9131_BLOB0_START_BLOCK 26
#define IPA9131_BLOB1_START_BLOCK 1023

struct part_block_off_t
{
	uint32_t partnum;
	uint32_t  nand_block_off;
};

static const struct part_block_off_t part_num_to_block_off[]={ 
								{.partnum=3, .nand_block_off=IPA9131_BLOB0_START_BLOCK,},
								{.partnum=5, .nand_block_off=IPA9131_BLOB1_START_BLOCK,},
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
    unsigned char * buf;

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
        fprintf(stderr,"gen_key_pair_csr returned error 0x%08X",ret);
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

/*Validate if in container has all the required tags, if in_container is empty then create a new container with all required tags
 * else generate missing tags(if possible) data and create a new container with all the missing tags (indicate consolidation would be required in this case)*/
static int validate_and_restore_container(struct container_field_t *in_data, struct container_field_t **new_data,uint8_t *need_consolidation)
{
    int ret = 0;
    struct container_field_t *pubkey_data = NULL , *privkey_data = NULL;

    if (!in_data )
    {
	ret = create_container(new_data);
    }
    else
    {
     
	if  ( NULL == ( privkey_data = find_container_field(RAW_CONTAINER_TAG_PRIVATE_KEY_BLOB, in_data)) )
        {
            /*No private key, return*/
            fprintf(stderr,"Private key blob not present in raw nand");	
            ret = -EFAULT;
	    goto cleanup;
        }

        
	if ( NULL == (pubkey_data = find_container_field(RAW_CONTAINER_TAG_PUBLIC_KEY, in_data)) )
        {
	    /*No pub key, return*/
            fprintf(stderr,"Private key blob not present in raw nand");
            ret = -EFAULT;
            goto cleanup;

        }

	if ( 0 != init_apk_black_key( privkey_data->value, privkey_data->length,pubkey_data->value,pubkey_data->length) )
	{
                fprintf(stderr,"Black key intialisation failed for sec engine");
		ret = -EFAULT;
		goto cleanup;
	}


        if ( NULL == find_container_field(RAW_CONTAINER_TAG_CSR,in_data) )
        {
            /*No csr, generate from keys*/
            if (0 != (ret = create_csr_container(pubkey_data,new_data)) )
            {
                fprintf(stderr,"Csr generation failed with existing key pair");
                goto cleanup;
            }

            *need_consolidation = 1;

        }
        
    }

cleanup:
    return ret;


}


static int update_nand_part(const struct container_field_t * fields, uint32_t partnum, uint32_t eraseblock_size)
{
    int ret, i;
    uint8_t * data;
    uint32_t len, start_addr;

    ret = 0;
    len = eraseblock_size;


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
        fprintf(stderr,"serialise_container");
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


    if (0 != (ret = read_raw_containers(raw_containers, num_raw_containers)))
    {
        errno = -ret;
        ret = 1;
        fprintf(stderr,"read_raw_containers");
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
                    /*Earse these nand partitions, so that on next reboot Key pair can be generated again*/
                    erase_nand(raw_containers, num_raw_containers);
                }
            }
            break;

        }
        else
        {
            fprintf(stderr,"validation and restoration of container failed mtd_part %d\n",raw_containers[i].partnum);
            if ( i == (num_raw_containers -1))
            {

                /*Failed validating all of the raw nand containers,
                * Earse these nand partitions, so that on next reboot Key pair can be generated again*/
                erase_nand(raw_containers, num_raw_containers);
                break;
            }
            else
            {
                continue;
            }
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
        return CMD_RET_SUCCESS;
    else
        return CMD_RET_FAILURE;


}



U_BOOT_CMD(restore_raw_container, 9, 0, do_restore_container,
		"Excercise restore_raw_container",
		"-m <mtd-part-num>"
		"up to 4 mtd part num can be provided"
	  );
