#include "secboot.h"

#include "key.h"

#include <common.h>
#include <asm-generic/errno.h>
#include "ipa73xx_fuse.h"

static int secboot = 0;
static int prodmode = 0;
static char ethaddr_str[18] = "";


static void dump_sec_status(void)
{
    printf("Ethaddr in fuses     : %s\n", ethaddr_str[0] ? ethaddr_str : "Not set");
    printf("Secure Boot Flag     : %s\n", secboot ? "Set" : "Not set");
    printf("Production Mode flag : %s\n", prodmode ? "Set" : "Not set");    
    
    printf("This is a %s board\n", prodmode ? "production" : (secboot ? "test" : (ethaddr_str[0] ? "development" : "blank"))); 
    if ( prodmode && ((!secboot) || (!ethaddr_str[0])) )
    {
        printf("WARNING, invalid setting, secure boot and Ethaddr should both be set for production board\n");
    }
    else if (secboot && (!ethaddr_str[0]))
    {
        printf("WARNING, invalid setting, Ethaddr should be set for test board\n");
    }
}


int load_security_requirements(void)
{
    int ret = 0;
    
    if (read_ethaddr_from_fuses(ethaddr_str, 0))
    {
        setenv("ethaddr", ethaddr_str);
        printf("ethaddr set from fuses\n");
    }
    
    secboot = read_secure_boot_fuse();
    prodmode = read_production_mode_fuse();
    dump_sec_status();

    if (secboot)
    {
#if 0
        /* TODO: use this block once we agree on test mode */
        if (0 != (ret = require_key(devmode ? "dev" : (tstmode ? "tst" : "ipaoem0"), "conf")))
        {
            goto cleanup;
        }

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
        0 != strcmp(argv[1], "blank"))
    {
        return CMD_RET_USAGE;
    }

    if (0 == strcmp(argv[1], "prodmode"))
    {
        return CMD_RET_SUCCESS;
    }

    if (0 == strcmp(argv[1], "testmode") && (!prodmode) )
    {
        return CMD_RET_SUCCESS;
    }

    if (0 == strcmp(argv[1], "devmode") && ( (!prodmode) && (!secboot) ) )
    {
        return CMD_RET_SUCCESS;
    }

    if (0 == strcmp(argv[1], "blank") && ( (!prodmode) && (!secboot) && (!ethaddr_str[0]) ) )
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
    return (prodmode);
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

    if (0 != strcmp(argv[1], "prodmode") &&
        0 != strcmp(argv[1], "testmode") &&
        0 != strcmp(argv[1], "devmode") &&
        0 != strcmp(argv[1], "blank"))
    {
        return CMD_RET_USAGE;
    }

    if (0 == strcmp(argv[1], "prodmode"))
    {
        prodmode = 1;
        secboot = 1;
        return CMD_RET_SUCCESS;
    }

    if (0 == strcmp(argv[1], "testmode"))
    {
//        if (prodmode == 1)
//        {   /* Actually couldn't run this command in prod mode! */
//            return CMD_RET_USAGE;
//        }
        prodmode = 0; // Temp
        secboot = 1;
        return CMD_RET_SUCCESS;
    }
    
    if (0 == strcmp(argv[1], "devmode"))
    {
//        if ( (prodmode == 1) || (secboot == 1) )
//        {
//            return CMD_RET_USAGE;
//        }

        prodmode = 0; // Temp
        secboot = 0;  // Temp
        return CMD_RET_SUCCESS;
    }

    /* ALL THIS BIT IS TEMP! */
    if (0 == strcmp(argv[1], "blank"))
    {
        prodmode = 0;
        secboot = 0;
        ethaddr_str[0] = 0;
        return CMD_RET_SUCCESS;
    }
    
    return CMD_RET_FAILURE;
}

U_BOOT_CMD(
    lie, 2, 2, do_lie,
    "Lie about the secure boot mode.",
    "<prodmode|tstmode|devmode> - force blank, development, test or production mode"
);
#endif
