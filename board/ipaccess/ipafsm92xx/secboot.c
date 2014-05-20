#include "secboot.h"

#include "key.h"

#include <common.h>
#include <fuse.h>
#include <asm-generic/errno.h>




#define SECURE_BOOT_ENABLE_BANK_ADDRESS 0x4318
#define SECURE_BOOT_ENABLE_WORD 0
#define DEVELOPMENT_MODE_ENABLE_BANK_ADDRESS 0x4320
#define DEVELOPMENT_MODE_ENABLE_WORD 0
#define TEST_MODE_ENABLE_BANK_ADDRESS 0x4328
#define TEST_MODE_ENABLE_WORD 0



static u8 secboot = 0;
static u8 devmode = 0;
static u8 tstmode = 0;
/* static u8 seccust = 0; */



int load_security_requirements()
{
    u32 secboot_lsb;
    u32 devmode_lsb;
    u32 tstmode_lsb;
    int ret;

    if (0 != (ret = fuse_sense(SECURE_BOOT_ENABLE_BANK_ADDRESS, SECURE_BOOT_ENABLE_WORD, &secboot_lsb)))
    {
        goto cleanup;
    }

    if (0 != (ret = fuse_sense(DEVELOPMENT_MODE_ENABLE_BANK_ADDRESS, DEVELOPMENT_MODE_ENABLE_WORD, &devmode_lsb)))
    {
        goto cleanup;
    }

    if (0 != (ret = fuse_sense(TEST_MODE_ENABLE_BANK_ADDRESS, TEST_MODE_ENABLE_WORD, &tstmode_lsb)))
    {
        goto cleanup;
    }

    secboot = (((secboot_lsb & 0x000000FF) & 0x0020) >> 5);
    devmode = (((devmode_lsb & 0x000000FF) & 0x0020) >> 5);
    tstmode = (((tstmode_lsb & 0x000000FF) & 0x0020) >> 5);

    printf("Blank Board        : %s\n", (!secboot && !devmode) ? "Yes" : "No");
    printf("Secure Boot Enabled: %s\n", secboot ? "Yes" : "No");
    printf("Development Mode   : %s\n", devmode ? "Yes" : "No");
    printf("Test Mode          : %s\n", tstmode ? "Yes" : "No");

    if (secboot)
    {
#if 0
        /* TODO: use this block once we agree on test mode */
        if (0 != (ret = require_key(devmode ? "dev" : (tstmode ? "tst" : "ipaoem0"), "conf")))
        {
            goto cleanup;
        }
#else
        /* TODO: remove this block once we agree on test mode */
        if (0 != (ret = require_key(devmode ? "dev" : "ipaoem0", "conf")))
        {
            goto cleanup;
        }
#endif
    }

cleanup:
    return ret;
}




#if defined(CONFIG_CMD_SECPARM)

int do_secparm(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    if (1 == argc)
    {
        printf("Secure Boot Enabled: %s\n", secboot ? "Yes" : "No");
        printf("Development Mode Enabled: %s\n", devmode ? "Yes" : "No");
        printf("Test Mode Enabled: %s\n", tstmode ? "Yes" : "No");
        return CMD_RET_SUCCESS;
    }

    if (2 != argc)
    {
        return CMD_RET_USAGE;
    }

    if (0 != strcmp(argv[1], "secboot") &&
        0 != strcmp(argv[1], "devmode") &&
        0 != strcmp(argv[1], "tstmode"))
    {
        return CMD_RET_USAGE;
    }

    if (0 == strcmp(argv[1], "secboot") && secboot)
    {
        return CMD_RET_SUCCESS;
    }

    if (0 == strcmp(argv[1], "devmode") && devmode)
    {
        return CMD_RET_SUCCESS;
    }

    if (0 == strcmp(argv[1], "tstmode") && tstmode)
    {
        return CMD_RET_SUCCESS;
    }

    return CMD_RET_FAILURE;
}

int silent_mode_enabled(void)
{
    // If secure boot set and not in development mode then we require silence.
    return (secboot && (0 == devmode));
}

static int do_silent_mode_enabled(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    int silence = silent_mode_enabled();

    if ( silence )
        return CMD_RET_SUCCESS;
    else
        return CMD_RET_FAILURE;
}

U_BOOT_CMD(
    silent_mode_enabled, 1, 0, do_silent_mode_enabled,
    "Determines if silent mode should be enabled",
    ""
);

U_BOOT_CMD(
    secparm, 2, 1, do_secparm,
    "report security parameters",
    "report security parameters to screen\n"
    "secparm <secboot|devmode|tstmode> - report on particular security flags via exit status"
);
#endif

#if defined(CONFIG_CMD_LIE)
int do_lie(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    if (2 != argc)
    {
        return CMD_RET_USAGE;
    }

    if (0 != strcmp(argv[1], "secboot") &&
        0 != strcmp(argv[1], "devmode"))
    {
        return CMD_RET_USAGE;
    }

    if (0 == strcmp(argv[1], "secboot"))
    {
        secboot = 1;
        devmode = 0;
        tstmode = 0;
        return CMD_RET_SUCCESS;
    }

    if (0 == strcmp(argv[1], "devmode"))
    {
        if (secboot && !devmode)
        {
            return CMD_RET_USAGE;
        }

        secboot = 1;
        devmode = 1;
        tstmode = 0;
        return CMD_RET_SUCCESS;
    }

    return CMD_RET_FAILURE;
}

U_BOOT_CMD(
    lie, 2, 2, do_lie,
    "Lie about the secure boot mode.",
    "<devmode|secboot> - force development mode or production secure boot on - you can only elevate the current setting"
);
#endif
