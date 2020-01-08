#include <common.h>
#include <bootcount.h>
#include <linux/compiler.h>

void bootcount_store(ulong a)
{

    uint8_t buf = (a & 0xff);
    int ret;
    char command_buffer[100];
    char *partition=NULL;

    if (NULL == (partition=getenv("userdata_pnum")))
        partition = "0xE"; //use default
    
    snprintf(command_buffer,sizeof(command_buffer),"ext4write mmc 0:%s 0x%x /bootcount 1",partition,&buf);
    ret = run_command(command_buffer,0);
    if (ret != 0)
        puts("Error writing bootcount\n");
}

ulong bootcount_load(void)
{
    uint8_t buf = 1;
    int ret;
    char command_buffer[100];
    char *partition=NULL;
    
    if (NULL == (partition=getenv("userdata_pnum")))
        partition = "0xE"; //use default


    snprintf(command_buffer,sizeof(command_buffer),"ext4load mmc 0:%s 0x%x /bootcount 1",partition,&buf);
    ret = run_command(command_buffer,0);

    if (ret != 0)
    {
        puts("Error loading bootcount\n");
        return 0;
    }

#if defined(CONFIG_BOOTCOUNT_FSM90XX_MODULUS)
    return buf % CONFIG_BOOTCOUNT_FSM90XX_MODULUS;
#else
    return buf;
#endif
}

static int do_bootcount_combined_exceeded(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    ulong bc = bootcount_load();

    if ( bc > (CONFIG_BOOTCOUNT_LIMIT_COUNT * 2) )
        return CMD_RET_SUCCESS;
    else
        return CMD_RET_FAILURE;
}


U_BOOT_CMD(
    bootcount_combined_exceeded, 1, 0, do_bootcount_combined_exceeded,
    "Determines if the boot count limit has been exceeded for both images",
    ""
);

