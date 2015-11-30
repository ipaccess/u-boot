#include <common.h>
#include <compiler.h>
#include <asm/errno.h>
#include <asm/io.h>
#include <asm/immap_85xx.h>
#include <fsl_sec.h>
#include "characterisation.h"
#include "sec.h"

#define SECMON_CHECK_STATE 0x00000900
void set_sec_state_to_fail(void)
{
       sec_out_be32(SECMON_HPCOMR,0x00000200);
}

void set_final_sec_state(void)
{
        lock_out_registers();

        if ( characterisation_is_specials_mode() )
        {
                /*Transition to soft fail state*/
                set_sec_state_to_fail();
        }

        /*Transition to appropriate state
         * for Specials causes: soft-fail->non-secure
         * for Prod/dev/test causes : Trusted-->secure*/
        sec_out_be32(SECMON_HPCOMR,0x00000001);

        /*Lock transition back to trusted state from secure state*/
        sec_out_be32(SECMON_HPCOMR,0x00000002);
}

void wait_until_out_of_check_state(void)
{
    while ( (sec_in_be32(SECMON_HPSR) & 0x00000F00) == SECMON_CHECK_STATE )
    {
        udelay(10000);
    }
}

#if defined CONFIG_CMD_SEC_STATE_CHANGE
int do_change_sec_state(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
        set_final_sec_state();
        return CMD_RET_SUCCESS;
}

U_BOOT_CMD(change_sec_state, 1, 0, do_change_sec_state,
                "Change security engine state",
                "change_sec_state"
          );
#endif
