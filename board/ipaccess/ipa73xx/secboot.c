#include "secboot.h"

#include "key.h"

#include <common.h>
#include <asm-generic/errno.h>
#include "ipa73xx_fuse.h"

#define __asmeq(x, y)  ".ifnc " x "," y " ; .err ; .endif\n\t"
#define SMC_GET_ACTIVE_RSM 6

/*RSM values returned by TZ*/
#define RSM_A_PROD          0x8
#define RSM_A_DEV           0x4
#define RSM_A_SPECIALS      0x2
#define RSM_A_FUSE          0x0
#define RSM_A_TEST          0x5


static char ethaddr_str[18] = "";

typedef enum {
    BT_DEVEL,
    BT_SPECIAL,
    BT_TEST,
    BT_PROD
} board_type_t;


board_type_t g_board_type = BT_PROD;
unsigned int secboot;


static unsigned int __smc(unsigned int command, unsigned int buff, unsigned int buff_len)
{
    register unsigned int r0 __asm("r0") = command;
    register unsigned int r1 __asm("r1") = buff;
    register unsigned int r2 __asm("r2") = buff_len;
    /*Invalidate and flush d-cache*/
    asm ( "mcr     p15, 0, r6, c7, c14, 0");
    asm volatile (
            __asmeq("%0", "r0")
            __asmeq("%1", "r0")
            __asmeq("%2", "r1")
            __asmeq("%3", "r2")
            ".arch armv7-a\n"
            ".arch_extension sec\n"
            "smc #0 @ switch to secure world\n"
            : "=r" (r0)
            : "r" (r0), "r" (r1), "r" (r2)
            );

    return r0;

}

unsigned int smc_get_active_rsm(void)
{
    return __smc(SMC_GET_ACTIVE_RSM,0,0);
}


static void dump_sec_status(void)
{
    printf("Ethaddr in fuses  : %s\n", ethaddr_str[0] ? ethaddr_str : "Not set");
    printf("Secure Boot Flag  : %s\n", secboot ? "Set" : "Not set");
    printf("Security Mode     : %s\n", (BT_PROD == g_board_type) ? "production" : ((BT_TEST == g_board_type) ? "test" : ((BT_DEVEL == g_board_type) ? "development" : "specials")));    
    
}


void tz_update_operating_mode(void)
{
    switch (smc_get_active_rsm())
    {
        case RSM_A_PROD:
            g_board_type = BT_PROD;
            break;
        case RSM_A_SPECIALS:
            g_board_type = BT_SPECIAL;
            break;
        case RSM_A_DEV:
            g_board_type = BT_DEVEL;
            break;
        case RSM_A_TEST:
            g_board_type = BT_TEST;
            break;
        case RSM_A_FUSE:
        default:
            /*Nothing to do, RSM should remain as it was initialised from fuses*/
            break;
    }
}

int load_security_requirements(void)
{
    int ret = 0;
    
    if (read_ethaddr_from_fuses(ethaddr_str, 0))
    {
        setenv("ethaddr", ethaddr_str);
    }
    
    secboot = read_secure_boot_fuse();

    /*Read fuses to derive board RSM*/
    if (read_production_mode_fuse())
        g_board_type = BT_PROD;
    else if (read_development_mode_fuse())
        g_board_type = BT_DEVEL;
    else if (read_specials_mode_fuse())
        g_board_type = BT_SPECIAL;
    else if (!secboot)
        /*secboot bit not fused and no RSM mode bit fused, consider this as development*/
        g_board_type = BT_DEVEL;
    else
        g_board_type = BT_PROD; 

    /*Overide  RSM if tz has different active RSM */
    tz_update_operating_mode(); 
   
    //If devmode, then see this needs to be set test mode on the basis of filesystem flag.
    if ( (BT_DEVEL == g_board_type))
    {
        mtdparts_init();

        if ( (0 == (ubi_part("FS", NULL))) && (0 == ubifs_mount("ubi0:factory_data")) && (0 == ubifs_ls("test-mode")) )
        {
            g_board_type = BT_TEST;
        }
        
    }

    if (BT_PROD != g_board_type)
        dump_sec_status();

cleanup:
    return ret;
}




#if defined(CONFIG_CMD_SECPARM)

int do_secparm(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    if (1 == argc)
    {
        dump_sec_status();
        return CMD_RET_SUCCESS;
    }

    if (2 != argc)
    {
        return CMD_RET_USAGE;
    }

    if (0 != strcmp(argv[1], "prodmode") &&
        0 != strcmp(argv[1], "testmode") &&
        0 != strcmp(argv[1], "devmode") &&
        0 != strcmp(argv[1], "specmode"))
    {
        return CMD_RET_USAGE;
    }

    if (0 == strcmp(argv[1], "prodmode"))
    {
        return CMD_RET_SUCCESS;
    }

    if (0 == strcmp(argv[1], "testmode") && (BT_PROD != g_board_type) )
    {
        return CMD_RET_SUCCESS;
    }

    if (0 == strcmp(argv[1], "devmode") && (BT_PROD != g_board_type) && (BT_TEST != g_board_type) )
    {
        return CMD_RET_SUCCESS;
    }

    if (0 == strcmp(argv[1], "specmode") && ( (BT_PROD != g_board_type) && (BT_TEST != g_board_type) && (BT_DEVEL != g_board_type)  ) )
    {
        return CMD_RET_SUCCESS;
    }
   
    return CMD_RET_FAILURE;
}


int silent_mode_enabled(void)
{
    /* If in production mode then we require silence - note that this does not check secboot
     * prodmode set and secboot clear is an invalid combination, so we default to most secure mode
     */  
    return (BT_PROD == g_board_type);
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
    "secparm <prodmode|devmode|testmode|blank>\n"
    "        return true if board mode equal or less than specified (may I boot at this level?)"
);
#endif

// Temporary open mode so we can force the board to be blank if things go wrong!
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
        } //Allow test mode to be downgranded to dev/spec 
        else if (0 == strcmp(argv[1], "devmode"))
        {
            g_board_type = BT_DEVEL;
            return CMD_RET_SUCCESS;
        }
        else if (0 == strcmp(argv[1], "spcmode"))
        {
            g_board_type = BT_SPECIAL;
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
