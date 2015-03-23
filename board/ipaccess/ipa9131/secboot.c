#include "secboot.h"

#include <common.h>
#include <fuse.h>
#include <asm-generic/errno.h>




typedef enum {
    BT_BLANK,
    BT_DEVEL,
    BT_SPECIAL,
    BT_TEST,
    BT_PROD
} board_type_t;


board_type_t g_board_type;

/*
 * Blank
 * Development Mode
 * Specials Mode
 * Test Mode
 * Production Mode
 */





int load_security_requirements()
{
    g_board_type = BT_DEVEL;
    return 0;
}




#if defined(CONFIG_CMD_SECPARM)
int do_secparm(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    if (1 == argc)
    {
/*
        printf("Secure Boot Enabled: %s\n", secboot ? "Yes" : "No");
        printf("Development Mode Enabled: %s\n", devmode ? "Yes" : "No");
        printf("Test Mode Enabled: %s\n", tstmode ? "Yes" : "No");
*/
        return CMD_RET_SUCCESS;
    }

    if (2 != argc)
    {
        return CMD_RET_USAGE;
    }

    if (0 != strcmp(argv[1], "prdmode") &&
        0 != strcmp(argv[1], "devmode") &&
        0 != strcmp(argv[1], "spcmode") &&
        0 != strcmp(argv[1], "blank") &&
        0 != strcmp(argv[1], "tstmode"))
    {
        return CMD_RET_USAGE;
    }

    if (0 == strcmp(argv[1], "prdmode") && BT_PROD == g_board_type)
    {
        return CMD_RET_SUCCESS;
    }

    if (0 == strcmp(argv[1], "devmode") && BT_DEVEL == g_board_type)
    {
        return CMD_RET_SUCCESS;
    }

    if (0 == strcmp(argv[1], "tstmode") && BT_TEST == g_board_type)
    {
        return CMD_RET_SUCCESS;
    }

    if (0 == strcmp(argv[1], "spcmode") && BT_SPECIAL == g_board_type)
    {
        return CMD_RET_SUCCESS;
    }

    if (0 == strcmp(argv[1], "blank") && BT_BLANK == g_board_type)
    {
        return CMD_RET_SUCCESS;
    }

    return CMD_RET_FAILURE;
}



int silent_mode_enabled(void)
{
    /* when in production mode we shall perform silent boot */
    return BT_PROD == g_board_type;
}



static int do_silent_mode_enabled(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    return silent_mode_enabled() ? CMD_RET_SUCCESS : CMD_RET_FAILURE;
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
    "secparm <prdmode|devmode|tstmode|spcmode|blank> - report on particular security flags via exit status"
);
#endif



#if defined(CONFIG_CMD_LIE)
int do_lie(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    if (2 != argc)
    {
        return CMD_RET_USAGE;
    }

    /*
     * If blank we can be anything
     * If special we are frozen
     * If development we can be test or production
     * If test we can be production
     */
    if (0 != strcmp(argv[1], "prdmode") &&
        0 != strcmp(argv[1], "tstmode") &&
        0 != strcmp(argv[1], "spcmode") &&
        0 != strcmp(argv[1], "devmode"))
    {
        return CMD_RET_USAGE;
    }

    if (BT_BLANK == g_board_type)
    {
        if (0 == strcmp(argv[1], "spcmode"))
        {
            g_board_type = BT_SPECIAL;
            return CMD_RET_SUCCESS;
        }
        else if (0 == strcmp(argv[1], "devmode"))
        {
            g_board_type = BT_DEVEL;
            return CMD_RET_SUCCESS;
        }
        else if (0 == strcmp(argv[1], "tstmode"))
        {
            g_board_type = BT_TEST;
            return CMD_RET_SUCCESS;
        }
        else if (0 == strcmp(argv[1], "prdmode"))
        {
            g_board_type = BT_PROD;
            return CMD_RET_SUCCESS;
        }
    }
    else if (BT_DEVEL == g_board_type)
    {
        if (0 == strcmp(argv[1], "tstmode"))
        {
            g_board_type = BT_TEST;
            return CMD_RET_SUCCESS;
        }
        else if (0 == strcmp(argv[1], "prdmode"))
        {
            g_board_type = BT_PROD;
            return CMD_RET_SUCCESS;
        }
    }
    else if (BT_TEST == g_board_type)
    {
        if (0 == strcmp(argv[1], "prdmode"))
        {
            g_board_type = BT_PROD;
            return CMD_RET_SUCCESS;
        }
    }

    return CMD_RET_FAILURE;
}

U_BOOT_CMD(
    lie, 2, 2, do_lie,
    "Lie about the secure boot mode.",
    "<spcmode|devmode|tstmode|prdmode> - force development mode or production secure boot on - you can only elevate the current setting"
);
#endif
