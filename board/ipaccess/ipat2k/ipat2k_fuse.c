#include <common.h>
#include <compiler.h>
#include <asm/errno.h>
#include <asm/io.h>
#include "ipat2k_fuse.h"


uint8_t ipat2k_fuse_read_loader_revocation()
{
    uint8_t buff[16], bit, i;
    unsigned int rc = MAX_REVOCATION_COUNT_VALUE;
    uint8_t mask=0x80;

    memset(buff,0,16);

    efuse_read_instance(REVOCATION_COUNT_EFUSE_INSTANCE,buff);

    for (i=0 ; i < 8;i++)
    {
        for (bit = 0;bit < 8 ;bit++)
        {
            if (buff[i] & (mask >> bit))
            {
                return rc;
            }
            --rc;
        }

    }
    return 0;
}

uint8_t ipat2k_fuse_read_application_revocation()
{
    uint8_t buff[16], bit, i;
    unsigned int rc = MAX_REVOCATION_COUNT_VALUE;
    uint8_t mask=0x80;

    memset(buff,0,16);

    efuse_read_instance(REVOCATION_COUNT_EFUSE_INSTANCE,buff);

    for (i=8 ; i < 16;i++)
    {
        for (bit = 0;bit < 8 ;bit++)
        {
            if (buff[i] & (mask >> bit))
            {
                return rc;
            }
            --rc;
        }

    }

    return 0;
}


void ipat2k_fuse_write_revocation_counts(uint8_t loader_rev_count,uint8_t app_rev_count)
{

    uint8_t buff[16],byte_index=0,bit_pos=0;
    memset(buff,0,16);

    if (loader_rev_count)
    {
        if (loader_rev_count > MAX_REVOCATION_COUNT_VALUE)
            loader_rev_count=MAX_REVOCATION_COUNT_VALUE;
        byte_index = 7 - (loader_rev_count-1)/8;
        bit_pos= (loader_rev_count-1)%8;
        buff[byte_index] = 1 << bit_pos;
    }

    if (app_rev_count)
    {
        if (app_rev_count > MAX_REVOCATION_COUNT_VALUE)
            app_rev_count=MAX_REVOCATION_COUNT_VALUE;
        byte_index = 15 - (app_rev_count-1)/8;
        bit_pos= (app_rev_count-1)%8;
        buff[byte_index] = 1 << bit_pos;
    }

    if (loader_rev_count || app_rev_count)
        efuse_write_instance(REVOCATION_COUNT_EFUSE_INSTANCE,buff);

}

uint8_t ipat2k_is_secure_boot()
{
    uint8_t buff[16];
    if (ipat2k_is_board_fused())
    {
        memset(buff,0,16);
        efuse_read_instance(INTEL_DEFINED_EFUSE_INSTANCE,buff);
        if ( buff[3] & 0x80 )
            return 1;
    }

    return 0;
}


uint8_t ipat2k_is_board_fused()
{
    uint8_t buff[16], i;
    efuse_read_instance(AP_CHARACTERISATION_EFUSE_INSTANCE,buff);
    for(i=0 ; i < 16; i++)
        if (buff[i] != 0)
            return 1;

    return 0;
}


static int do_ipat2k_secure(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{

    int prompt_to_user = 1, i = 1;
    int disable_jtag = 1;
    uint8_t efuse0_buff[16],efuse2_buff[16];
    /*CA TMSK public key hash*/
    uint8_t oem_sig_pub_key_hash[32]={ 0x91,0x91,0x70,0x65,0xAA,0xBD,0x72,0x77,0x92,0xF5,0x1C,0xE6,0xE7,0xB3,0xA0,0x4C,
                                       0xD9,0x4E,0x52,0x74,0x96,0xCC,0x0A,0x4F,0xCE,0x7D,0x51,0x85,0x3A,0x56,0x4D,0x17 };

    while( i < argc )
    {
        if ( 0 == strcmp(argv[i],"-no-prompt") )
        {
            prompt_to_user = 0;
        }
        else if ( 0 == strcmp(argv[i],"-full-dbg-permission") )
        {
            disable_jtag = 0;
        }
        else
        {
            return CMD_RET_USAGE;
        }

        i++;

    }

    if (!ipat2k_is_board_fused())
    {
        fprintf(stderr,"\nInvalid operation: Board not characterised, fuse characterisation data first\n");
        goto error;
    }

    if ( prompt_to_user )
    {
        fprintf(stdout,"Do you wish to go ahead and make this as secure boot board? this will permanently blow fuses\n");
        fprintf(stdout, "Type 'Y' to proceed: ");


        if ('Y' != getc())
        {
            fprintf(stdout, "\nUser cancelled operation\n");
            goto error;
        }

        fprintf(stdout,"\nYou're sure? ");
        fprintf(stdout, "Type 'Y' to proceed: ");

        if ('Y' != getc())
        {
            fprintf(stdout,"\nUser cancelled operation\n");
            goto error;
        }

        fprintf(stdout,"\nAttempting to write the fuses\n");
    }


    efuse_write_instance(OEM_SIGN_KEY_HASH_PART_1_INSTANCE,&oem_sig_pub_key_hash[0]);
    efuse_write_instance(OEM_SIGN_KEY_HASH_PART_2_INSTANCE,&oem_sig_pub_key_hash[16]);

    memset(efuse0_buff,0,16);
    memset(efuse2_buff,0,16);

/*EFUSE INSTANCE_0 byte 0 masks*/
#define BOUNDARY_SCAN_DISBALE     0x01
#define TZ_ENABLE                 0x02
#define BLOCK_JTAG_CORESIGHT      0x04

/*EFUSE INSTANCE_0 byte 1 masks*/
#define AUTHENTICATE_BOOT_CODE    0x02
#define DEBUG_MODE                0x01
#define KEY_SIZE_2K               0x08
#define KEY_IN_HEADER		  0x10

/*EFUSE INSTANCE_0 sceure boot enable*/
#define SECURE_BOOT_ENABLE        0x80

/*EFUSE INSTANCE_2 byte 15 masks*/	
#define BLOCK_INVASIVE_DEBUG      0x40
#define BLOCK_NON_INVASIVE_DEBUG  0x20

    efuse0_buff[0] |= TZ_ENABLE;
    efuse0_buff[1] |= AUTHENTICATE_BOOT_CODE | DEBUG_MODE | KEY_SIZE_2K | KEY_IN_HEADER;
    efuse0_buff[3] |= SECURE_BOOT_ENABLE;

    if (disable_jtag)
    {
        /*Boundary scan disable*/
        efuse0_buff[0]  |= BOUNDARY_SCAN_DISBALE|BLOCK_JTAG_CORESIGHT;
        efuse2_buff[15] |= BLOCK_INVASIVE_DEBUG | BLOCK_NON_INVASIVE_DEBUG;

    }

    /*Set last time program bits for INTEL_DEFINED_EFUSE_INSTANCE,OEM_SIGN_KEY_HASH_PART_1_INSTANCE,OEM_SIGN_KEY_HASH_PART_2_INSTANCE*/
    efuse2_buff[byte_index_from_instance_num(INTEL_DEFINED_EFUSE_INSTANCE)] |= 1 << bit_shift_from_instance_num(INTEL_DEFINED_EFUSE_INSTANCE);
    efuse2_buff[byte_index_from_instance_num(OEM_SIGN_KEY_HASH_PART_1_INSTANCE)] |= 1 << bit_shift_from_instance_num(OEM_SIGN_KEY_HASH_PART_1_INSTANCE);
    efuse2_buff[byte_index_from_instance_num(OEM_SIGN_KEY_HASH_PART_2_INSTANCE)] |= 1 << bit_shift_from_instance_num(OEM_SIGN_KEY_HASH_PART_2_INSTANCE);

    efuse_write_instance(INTEL_DEFINED_EFUSE_INSTANCE,efuse0_buff);
    
    efuse_write_instance(LAST_TIME_PROG_EFUSE_INSTANCE,efuse2_buff);

    memset(efuse2_buff,0x0,16);
    memset(efuse2_buff,0xFF,6);
    efuse2_buff[6] = 0x0F;
    /*Disable access to all 51 fuse instances from external pins*/
    efuse_write_instance(DIS_EXTERNAL_PIN_EFUSE_INSTANCE,efuse2_buff);

    fprintf(stdout,"\nSuccess: secure boot enabled\n");

    return CMD_RET_SUCCESS;

error:
    return CMD_RET_FAILURE;

}

U_BOOT_CMD(ipat2k_go_secure, 3, 0, do_ipat2k_secure,
        "Prepare Board for secure boot",
        "ipat2k_go_secure <-no-prompt|-full-dbg-permission>"
        );

