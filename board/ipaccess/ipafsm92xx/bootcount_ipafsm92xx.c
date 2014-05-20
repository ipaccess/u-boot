#include "pm8058.h"

#include <common.h>

void bootcount_store(ulong a)
{
    if (0 != pm8058_write(0x0000002f, a & 0x000000ff))
    {
        debug("%s: pm8058_write failed\n", __func__);
    }
}

ulong bootcount_load(void)
{
    u8 r0;

    if (0 != pm8058_read(0x0000002f, &r0))
    {
        debug("%s: pm8058_read failed\n", __func__);
        r0 = 0;
    }

    return r0;
}

static int do_bootcount_combined_exceeded(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    ulong bc = bootcount_load();

    if ( bc > (CONFIG_BOOTCOUNT_LIMIT * 2) )
        return CMD_RET_SUCCESS;
    else
        return CMD_RET_FAILURE;
}


U_BOOT_CMD(
    bootcount_combined_exceeded, 1, 0, do_bootcount_combined_exceeded,
    "Determines if the boot count limit has been exceeded for both images",
    ""
);
