#include <common.h>
#include <netdev.h>
#include <asm/io.h>
#include <asm/arch/systimer.h>
#include <asm/arch/sysctrl.h>
#include <asm/arch/wdt.h>
#include "ipafsm92xx_nand.h"
#include "pm8058.h"
#include "tlmm.h"
#include "secboot.h"
#include "hwinfo.h"
//#include "console.h"

DECLARE_GLOBAL_DATA_PTR;

#if ! defined(FEMTO_GPIO_PS_HOLD)
#define FEMTO_GPIO_PS_HOLD 161
#endif

void s_init(void)
{

}

#if defined(CONFIG_SHOW_BOOT_PROGRESS)
void show_boot_progress (int progress)
{
    printf("Boot reached stage %d\n", progress);
}
#endif


int board_init(void)
{
    printf("%s\n", "Initialising ipafsm92xx board...");

    /* Setup machine type */
    gd->bd->bi_arch_number = MACH_TYPE_FSM9XXX_SURF;

    /* Setup kernel boot parameters address */
    gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

#if defined(CONFIG_STATUS_LED) && defined(STATUS_LED_BOOT)
    status_led_set (STATUS_LED_BOOT, STATUS_LED_ON);
#endif

    return 0;
}

int misc_init_r (void)
{
    if (0 != pm8058_init())
    {
        debug("%s: pm8058_init failed\n", __func__);
    }

    if (0 != load_security_requirements())
    {
        printf("FATAL: %s\n", "Failed to load security requirements, cannot proceed");
        return -1;
    }

    if (silent_mode_enabled())
    {
        setenv("silent", "1");
        setenv("bootdelay", "0");
    }

    print_ipa_hardware_info();
    return 0;
}

int dram_init(void)
{
    gd->ram_size = CONFIG_SYS_SDRAM_SIZE;
    return 0;
}

void dram_init_banksize (void)
{
    gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
    gd->bd->bi_dram[0].size  = PHYS_SDRAM_1_SIZE;
}


////////////////////////////////////////////////////////////////////////

int board_eth_init(bd_t *bis)
{
	int rc = 0;
//#ifdef CONFIG_IPAFSM9XXX
//	rc = ipafsm9xxx_initialize(0, CONFIG_IPAFSM9XXX_BASE);
//#endif
	return rc;
}

//static struct ptable flash_ptable; 


void board_nand_init (void)
{
    //TODO need to pass nand_info[] in the same way as in palmtreo680 to a function
    //to populate the nand_chip structure to overide standard functions used in nand_set_defaults()
 
    //struct mtd_info* mtd = &nand_info[0];
    //struct nand_chip* nand = &fsm92xx_nand_chip;

//TODO loop multiple NAND devices
    if (ipafsm92xx_nand_init())
        hang();
}

/* Use the Qualcomm watchdog to cause reset */
void reset_cpu(ulong addr)
{
    printf("%s: rebooting the board\n", __func__);

    if (0 != tlmm_func(FEMTO_GPIO_PS_HOLD, 0))
    {
        error("%s: tlmm_func failed\n", __func__);
        hang();
    }

    if (0 != tlmm_oe(FEMTO_GPIO_PS_HOLD, TLMM_OE_ENABLE))
    {
        error("%s: tlmm_oe failed\n", __func__);
        hang();
    }

    if (0 != tlmm_pull(FEMTO_GPIO_PS_HOLD, TLMM_PULL_NONE))
    {
        error("%s: tlmm_pull failed\n", __func__);
        hang();
    }

    if (0 != tlmm_drv(FEMTO_GPIO_PS_HOLD, TLMM_DRIVE_2MA))
    {
        error("%s: tlmm_drv failed\n", __func__);
        hang();
    }

    if (0 != tlmm_out(FEMTO_GPIO_PS_HOLD, 0))
    {
        error("%s: tlmm_drv failed\n", __func__);
        hang();
    }

    udelay(1000*1000*10);
    printf("%s: reboot failed!\n", __func__);
    hang();
}
