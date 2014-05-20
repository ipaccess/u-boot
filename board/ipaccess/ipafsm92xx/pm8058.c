#include "pm8058.h"

#include "ssbi.h"

#include <common.h>
#include <vsprintf.h>

#define PMIC_SSBI_BASE (SSBI2_0_BASE)

#define PMIC_SBI_CONFIG_REG         (0x0000004f)
#define PMIC_SBI_CONFIG_MODE_1          (0x00000000)
#define PMIC_SBI_CONFIG_MODE_2          (0x00000010)
#define PMIC_SBI_CONFIG_WIDTH(w)        ((w==10)?(0x00000040):(0x00000000))
#define PMIC_SBI_CONFIG_BANK_M          (0x00000006)
#define PMIC_SBI_CONFIG_BANK(b)         (((b)<<1)&0x00000006)

#define PMIC_SSBI_FALLBACK_MODE  (SSBI_MODE_2)
#define PMIC_SSBI_FALLBACK_WIDTH (10)


static u32 _mode;
static u32 _width;



int pm8058_init()
{
    debug("%s: initialising the PMIC\n", __func__);

    if (ssbi_ready(PMIC_SSBI_BASE) != 0)
    {
        if (ssbi_init(PMIC_SSBI_BASE) != 0)
        {
            debug("%s: ssbi_init failed\n", __func__);
            return -1;
        }

        _mode = PMIC_SSBI_FALLBACK_MODE;
        _width = PMIC_SSBI_FALLBACK_WIDTH;

        if (_mode == SSBI_MODE_2)
        {
            if (ssbi_write(PMIC_SSBI_BASE, PMIC_SBI_CONFIG_REG, PMIC_SBI_CONFIG_MODE_2 | PMIC_SBI_CONFIG_WIDTH(_width)) != 0)
            {
                debug("%s: ssbi_write failed\n", __func__);
                return -1;
            }
        }
        else
        {
            if (ssbi_write(PMIC_SSBI_BASE, PMIC_SBI_CONFIG_REG, PMIC_SBI_CONFIG_MODE_1) != 0)
            {
                debug("%s: ssbi_write failed\n", __func__);
                return -1;
            }
        }

        if (ssbi_mode_set(PMIC_SSBI_BASE, _mode, _width) != 0)
        {
            debug("%s: ssbi_mode_set failed\n", __func__);
            return -1;
        }
    }
    else
    {
        if (ssbi_mode_get(PMIC_SSBI_BASE, &_mode, &_width) != 0)
        {
            debug("%s: ssbi_mode_get failed\n", __func__);
            return -1;
        }
    }

    debug("%s: PMIC initialised\n", __func__);
    return 0;
}



static int _set_bank(u32 address)
{
    u32 v;
    u32 b;

    /*
     * Read the config register
     */
    if (ssbi_read(PMIC_SSBI_BASE, PMIC_SBI_CONFIG_REG, &v) != 0)
    {
        return -1;
    }

    /*
     * Select proper bank if in 8-bit mode
     */
    v &= ~PMIC_SBI_CONFIG_BANK_M;

    /*
     * Extrack bank from address
     */
    b = (address & 0x00000300) >> 8;

    if (ssbi_write(PMIC_SSBI_BASE, PMIC_SBI_CONFIG_REG, (v | PMIC_SBI_CONFIG_BANK(b))) != 0)
    {
        return -1;
    }

    return 0;
}



static int _read(u32 address, u32 * value)
{
    debug("%s: entering...\n", __func__);

    if (_mode == SSBI_MODE_1 || (_mode == SSBI_MODE_2 && _width == 8))
    {
        debug("%s: _mode == SSBI_MODE_1 || (_mode == SSBI_MODE_2 && _width == 8)\n", __func__);

        if (_set_bank(address) != 0)
        {
            debug("%s: _set_bank(0x%08x) failed\n", __func__, address);
            return -1;
        }

        debug("%s: before _set_bank, address is 0x%08x\n", __func__, address);
        address &= 0x000000ff;
        debug("%s: after _set_bank, address is 0x%08x\n", __func__, address);
    }

    debug("%s: performing ssbi_read...\n", __func__);
    return ssbi_read(PMIC_SSBI_BASE, (address & 0x000003ff), value);
}



static int _write(u32 address, u32 value)
{
    debug("%s: start _write(0x%08x, 0x%02x)\n", __func__, address, value);

    if (_mode == SSBI_MODE_1 || (_mode == SSBI_MODE_2 && _width == 8))
    {
        if (_set_bank(address) != 0)
        {
            return -1;
        }

        address &= 0x000000ff;
    }

    /*
     * Write to the register
     */
    debug("%s: tail calling ssbi_write\n", __func__); /* what's the mask about? */
    return ssbi_write(PMIC_SSBI_BASE, (address & 0x000003ff), value);
}



int pm8058_read(u32 address, u8 * value)
{
    u32 v;
    int ret;

    debug("%s: pm8058_read(0x%08x, <value>)\n", __func__, address);

    if (0 != (ret = _read(address, &v)))
    {
        debug("%s: _read failed\n", __func__);
        return ret;
    }

    *value = (v & 0x000000ff);
    debug("%s: _read success, value is 0x%02x\n", __func__, *value);
    return 0;
}



int pm8058_write(u32 address, u8 value)
{
    u32 v = value;
    debug("%s: pm8058_write(0x%08x, 0x%02x) - tail calling _write\n", __func__, address, value);
    return _write(address, v);
}




#if defined(CONFIG_CMD_PM8058)
static int do_pm8058(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    u8 r0;
    u32 address;
    u32 value;
    char * end;

    switch (argc)
    {
        case 3: /* pmic read <address> */
        {
            if (strcmp(argv[1], "read") != 0)
            {
                return CMD_RET_USAGE;
            }

            address = simple_strtoul(argv[2], &end, 16);

            if (*end)
            {
                return CMD_RET_USAGE;
            }

            debug("pm8058: parsed address 0x%08x\n", address);

            if (0 != pm8058_read(address, &r0))
            {
                return CMD_RET_FAILURE;
            }

            printf("pm8058[0x%08x]: 0x%02x (", address, r0);
            printf("%d", (r0 & 0x80) >> 7);
            printf("%d", (r0 & 0x40) >> 6);
            printf("%d", (r0 & 0x20) >> 5);
            printf("%d", (r0 & 0x10) >> 4);
            printf("%d", (r0 & 0x08) >> 3);
            printf("%d", (r0 & 0x04) >> 2);
            printf("%d", (r0 & 0x02) >> 1);
            printf("%d", (r0 & 0x01) >> 0);
            printf("%s", ")\n");
            break;
        }
        case 4: /* pmic write <address> <value> */
        {
            if (strcmp(argv[1], "write") != 0)
            {
                return CMD_RET_USAGE;
            }

            address = simple_strtoul(argv[2], &end, 16);

            if (*end)
            {
                return CMD_RET_USAGE;
            }

            debug("pm8058: parsed address 0x%08x\n", address);

            value = simple_strtoul(argv[3], &end, 16);

            if (*end)
            {
                return CMD_RET_USAGE;
            }

            debug("pm8058: parsed value 0x%08x\n", value);
            r0 = value & 0x000000ff;
            debug("pm8058: masked value 0x%02x\n", r0);

            if (0 != pm8058_write(address, r0))
            {
                return CMD_RET_FAILURE;
            }

            break;
        }
        default:
        {
            return CMD_RET_USAGE;
        }
    }

    return CMD_RET_SUCCESS;
}


U_BOOT_CMD(
    pm8058, 4, 3, do_pm8058,
    "read and write PMIC registers",
    "read <address> - read the value of a register\n"
    "pm8058 write <address> <value> - write a value to a register"
);
#endif
