
#include <bootcount.h>
#include <linux/compiler.h>
#include <i2c.h>

void bootcount_store(ulong a)
{
    uint8_t buf = (a & 0xff);
    int ret;

    ret = i2c_write(CONFIG_BOOTCOUNT_EEPROM_ADDR,
                    CONFIG_BOOTCOUNT_IPA9131_OFFSET,
                    1,
                    &buf,
                    1);

    if (ret != 0)
        puts("Error writing bootcount\n");
}

ulong bootcount_load(void)
{
    uint8_t buf = 0;
    int ret;

    ret = i2c_read(CONFIG_BOOTCOUNT_EEPROM_ADDR,
                   CONFIG_BOOTCOUNT_IPA9131_OFFSET,
                   1,
                   &buf,
                   1);

    if (ret != 0)
    {
        puts("Error loading bootcount\n");
        return 0;
    }

#if defined(CONFIG_BOOTCOUNT_IPA9131_MODULUS)
    return buf % CONFIG_BOOTCOUNT_IPA9131_MODULUS;
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
