#include <common.h>
#include <linux/compiler.h>
#include <i2c.h>
#include "ipa9131_test_board.h"

int ipa9131_toggle_test_board(void)
{
    uint8_t buf = 0;
    int ret = 0;

    if ( -1 != (ret = ipa9131_is_test_board()) )
    {
        buf = !ret;
    }
    else
    {
        fprintf(stderr,"Error checking current status of board\n");
        return -1;
    }

    if ( 0 != i2c_write(CONFIG_CHARACTERISATION_EEPROM_ADDR,
                CONFIG_IPA9131_MISC_FLAGS_OFFSET,
                1,
                &buf,
                1) )
    {
        fprintf(stderr,"Error writing test board bit\n");
        return -1;
    }

    return 0;

}

int ipa9131_is_test_board(void)
{
    uint8_t buf = 0;

    if ( 0 != i2c_read(CONFIG_CHARACTERISATION_EEPROM_ADDR,
                CONFIG_IPA9131_MISC_FLAGS_OFFSET,
                1,
                &buf,
                1) )
    {
        fprintf(stderr,"Error Reading test board bit\n");
        return -1;
    }

    return (int) (buf & 0x01);
}

#if defined CONFIG_CMD_IPA9131_TOGGLE_TEST_BOARD

int do_ipa9131_toggle_test_board(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    int ret = 0;

    if (argc != 2)
        return CMD_RET_USAGE;

    if (0 == strcmp(argv[1], "status")) 
    {
        if ( -1 != (ret = ipa9131_is_test_board())) 
        {
            (ret == 1)?printf("Configured as Test Board\n"):printf("Not configured as Test Board\n");
            return CMD_RET_SUCCESS;
        }
        else
        {
            return CMD_RET_FAILURE;
        }

    }

    if (0 == strcmp(argv[1], "toggle"))
    {
        if ( 0 != ipa9131_toggle_test_board() )
        {
            return CMD_RET_FAILURE;
        }
        else
        {
            return CMD_RET_SUCCESS;
        }
    }

    return CMD_RET_USAGE;
}


U_BOOT_CMD(
        ipa9131_toggle_test_board, 2, 0, do_ipa9131_toggle_test_board,
        "Toggles the board as test board or displays it's status ",
        "toggle|status"
        );

#endif
