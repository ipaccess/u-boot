/* #include "pm8058.h" */

#include <common.h>

void bootcount_store(ulong a)
{
    /* Implement something to store bootcount */
}

ulong bootcount_load(void)
{
    u8 r0 = 1;

    /* Implement something to read bootcount 
       Always retruns 1 for now*/
    
    return r0;
}

static int do_bootcount_combined_exceeded(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    ulong bc = bootcount_load();
    
    printf("*** Combined bootcount = %u ***\n", bc);
    
    if ( bc > (CONFIG_BOOTCOUNT_LIMIT * 2) )
    {
        printf("!!! Maximum combined bootcount exceeded - halting !!!\n");
        return CMD_RET_SUCCESS;
    }
    else
    {
        return CMD_RET_FAILURE;
    }
}


U_BOOT_CMD(
    bootcount_combined_exceeded, 1, 0, do_bootcount_combined_exceeded,
    "Determines if the boot count limit has been exceeded for both images",
    ""
);
