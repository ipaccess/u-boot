/*****************************************************************************
 * $picoChipHeaderSubst$
 *****************************************************************************/

/*!
* \file pc7302.c
* \brief Various useful functions for use on a PC7302 Platform.
*
* Copyright (c) 2006-2011 Picochip Ltd
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
* All enquiries to support@picochip.com
*/

/* Includes ---------------------------------------------------------------- */
#include <common.h>
#include <asm/arch/timer.h>
#include <asm/arch/utilities.h>
#include <asm/arch/mux.h>
#include <asm/arch/pc3xxgpio.h>
#include <netdev.h>

/* Macros ------------------------------------------------------------------ */

/* Constants --------------------------------------------------------------- */
DECLARE_GLOBAL_DATA_PTR;

/* Prototypes--------------------------------------------------------------- */
/*!
 *
 * Start a timer in free running mode
 *
 * \param timer, the timer to start
 *
 */
static void pc302_timer_start(int timer);

/* Functions --------------------------------------------------------------- */

/*****************************************************************************
 *
 * show_boot_progress()
 *
 * Purpose: Indicate booting progress
 *
 * Note: see U-Boot README for a list of 'progress' values.
 *
 *****************************************************************************/
#if defined(CONFIG_SHOW_BOOT_PROGRESS)
void show_boot_progress(int progress)
{
	printf("Boot reached stage %d\n", progress);
}
#endif

/*****************************************************************************
 *
 * board_init()
 *
 * Purpose: Hardware platform initialisation functions
 *
 * Returns: 0 - Success
 *
 *****************************************************************************/
int board_init (void)
{
    /* Setup some kernel boot parameters */
    gd->bd->bi_boot_params = 0x00000100;

    /* Setup the machine type */
    gd->bd->bi_arch_number = MACH_TYPE_PC7302;

    gd->flags = 0;

    /* Enable the Instruction Cache */
    icache_enable ();

    /* Start timer #0 */
    pc302_timer_start(0);

    /* Initialise the gpio muxing library */
    pc3xx_muxing_init();

    /* Initialise the gpio library */
    pc3xx_gpio_init();

    return 0;
}

/*****************************************************************************
 *
 * checkboard()
 *
 * Purpose: Display some useful hardware platform information.
 *
 * Returns: 0 - Success
 *
 *****************************************************************************/
int checkboard (void)
{
    printf("Board: Picochip "PICOCHIP_PLATFORM" \n");

    return 0;
}

/*****************************************************************************
 *
 * print_cpuinfo()
 *
 * Purpose: Display some useful info about the cpu we are running on.
 *
 * Returns: 0 - Success
 *
 *****************************************************************************/
int print_cpuinfo(void)
{
    unsigned int device_id, device_rev;
    unsigned int die_id[4];

    /* What device are we running on ? */
    printf("CPU:   ");

    device_id = pc302_read_device_id();         /* Read the device id */
    device_rev = pc302_read_device_revision();  /* Read the revision code */
    pc302_read_die_id_number(&die_id[0]);       /* Read the die id */

    switch (device_id)
    {
        case PC302_DEVICE_ID:
        {
            printf("PC302 Rev %04d\n", device_rev);
            break;
        }
        case PC312_DEVICE_ID:
        {
            printf("PC312 Rev %04d\n", device_rev);
            break;
        }
        case PC313_DEVICE_ID:
        {
            printf("PC313 Rev %04d\n", device_rev);
            break;
        }
        case PC323_DEVICE_ID:
        {
            printf("PC323 Rev %04d\n", device_rev);
            break;
        }
        case PC333_DEVICE_ID:
        {
            printf("PC333 Rev %04d\n", device_rev);
            break;
        }
        default:
        {
            printf("Unknown !\n");
        }
    }

    return 0;
}

/*****************************************************************************
 *
 * misc_init_r()
 *
 * Purpose: Miscellaneous platform dependent initialisations
 *
 * Returns: 0 - Success
 *
 *****************************************************************************/
int misc_init_r (void)
{
    /* Not used right now, function template left here as a place holder */
    return 0;
}

/*****************************************************************************
 *
 * dram_init()
 *
 * Purpose: Initialize the DDR SDRAM info in the board data structure
 *
 * Returns: 0 - Success
 *
 *****************************************************************************/
int dram_init (void)
{
    gd->ram_size =
        get_ram_size((long *)PHYS_SDRAM_1, PHYS_SDRAM_1_SIZE);

    return 0;
}

void dram_init_banksize(void)
{
    gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
    gd->bd->bi_dram[0].size =
        get_ram_size((long *)PHYS_SDRAM_1, PHYS_SDRAM_1_SIZE);
}

static void pc302_timer_start(int timer)
{
    /* Make sure the timer is disabled */
    pc302_write_to_register( ( CONFIG_SYS_TIMERBASE +
                               TIMERNCONTROLREGOFFSET (timer) ),
                              0 );

    /* Initialise the timer to all 1's.
     * We do this  because we want to run
     * the timer in free running mode.
     */
    pc302_write_to_register( ( CONFIG_SYS_TIMERBASE +
                               TIMERNLOADCOUNTREGOFFSET (timer) ),
                              0xFFFFFFFF );

    /* Start the timer in free running mode */
    pc302_write_to_register( ( CONFIG_SYS_TIMERBASE +
                               TIMERNCONTROLREGOFFSET (timer) ),
                             ( TIMERINTERRUPTMASK | TIMERENABLE ) );
}

/*****************************************************************************
 *
 * board_eth_init()
 *
 * Purpose: Initialize the Ethernet controller
 *
 * Returns: 0 - Success
 *
 *****************************************************************************/

int board_eth_init(bd_t *bis)
{
    int ret = -1;

#ifdef CONFIG_DW_EMAC
    ret = pc302_eth_register(bis);
#endif

    return ret;
}
