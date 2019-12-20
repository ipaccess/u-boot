#include <common.h>
#include <netdev.h>
#include <asm/io.h>
#include <asm/arch/systimer.h>
#include <asm/arch/sysctrl.h>
#include <asm/arch/wdt.h>
#include "iomap.h"
DECLARE_GLOBAL_DATA_PTR;

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
    printf("%s\n", "Initialising ipafsm90xx board...");

    /* Setup machine type */
    gd->bd->bi_arch_number = MACH_TYPE_FSM9XXX_SURF;

    /* Setup kernel boot parameters address */
    gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;  //TODO
    
    //todo call from dram_init_banksize
    gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
    gd->bd->bi_dram[0].size  = PHYS_SDRAM_1_SIZE;

#if defined(CONFIG_STATUS_LED) && defined(STATUS_LED_BOOT)
    //status_led_set (STATUS_LED_BOOT, STATUS_LED_ON);
    printf("%s TODO set status led\n",__func__)
#endif

    return 0;
}

int misc_init_r (void)
{
    //print_ipa_hardware_info(); //TODO
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
    //TODO might be able to do this on this board.
    return rc;
}

int board_mmc_init(bd_t *bis)
{
    //TODO: place this somwhere else better
    platform_clock_init();
    return target_mmc_sdhci_init();
}


void reset_cpu(ulong addr)
{
    /* Drop PS_HOLD for MSM */
    writel(0x00, MPM2_MPM_PS_HOLD);
    mdelay(5000);
    printf("Shutdown failed\n");

}

#if 0 //board hangs with this at the moment. But it will be good to enable this for faster bootup.
void enable_caches(void)
{
    dcache_enable();
}
#endif
