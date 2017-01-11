#include "secboot.h"

#include <common.h>
#include <fuse.h>
#include <asm-generic/errno.h>

#include "characterisation.h"




typedef enum {
    BT_DEVEL,
    BT_SPECIAL,
    BT_TEST,
    BT_PROD
} board_type_t;


board_type_t g_board_type;




int load_security_requirements()
{
    if (characterisation_is_production_mode())
        g_board_type = BT_PROD;
    else if (characterisation_is_test_mode())
        g_board_type = BT_TEST;
    else if (characterisation_is_development_mode())
        g_board_type = BT_DEVEL;
    else if (characterisation_is_specials_mode())
        g_board_type = BT_SPECIAL;
    else
        g_board_type = BT_PROD;

    return 0;
}




int silent_mode_enabled(void)
{
    return BT_PROD == g_board_type;
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

    return CMD_RET_FAILURE;
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

    if (BT_DEVEL == g_board_type)
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



static int  __smc(u32 command, u32 buff1, u32 buff1_len,u32 buff2,u32 buff2_len,u32 buff3,u32 buff3_len) {

    register u32 r0 asm("r0") = command;
    register uint32_t r1 __asm("r1") = buff1;
    register uint32_t r2 __asm("r2") = buff1_len;
    register uint32_t r3 __asm("r3") = buff2;
    register uint32_t r4 __asm("r4") = buff2_len;
    register uint32_t r5 __asm("r5") = buff3;
    register uint32_t r6 __asm("r6") = buff3_len;

    asm volatile(
            __asmeq("%0", "r0")
            __asmeq("%1", "r0")
            __asmeq("%2", "r1")
            __asmeq("%3", "r2")
            __asmeq("%4", "r3")
            __asmeq("%5", "r4")
            __asmeq("%6", "r5")
            __asmeq("%7", "r6")
            ".arch armv7-a\n"
            "smc #0 @ switch to secure world\n"
            : "=r" (r0)
            : "r" (r0), "r" (r1), "r" (r2), "r" (r3), "r" (r4), "r" (r5), "r" (r6)
            );

    return r0;
}

/*command value as defined and used in microloader code*/
#define SMC_GET_ACTIVE_RSM 6
int smc_get_active_rsm(void)
{
        return __smc(SMC_GET_ACTIVE_RSM,0,0,0,0,0,0);
}


