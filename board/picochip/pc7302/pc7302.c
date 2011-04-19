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
#include <asm/arch/mux.h>
#include <asm/arch/utilities.h>
#include <asm/arch/picoxcell_gpio.h>

/* Macros ------------------------------------------------------------------ */

/* Constants --------------------------------------------------------------- */
DECLARE_GLOBAL_DATA_PTR;

/* Prototypes--------------------------------------------------------------- */

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
void show_boot_progress (int progress)
{
	printf ("Boot reached stage %d\n", progress);
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
	picoxcell_timer_start (0);

	/* Initialise the gpio muxing library */
	picoxcell_muxing_init ();

	/* Initialise the gpio library */
	picoxcell_gpio_init ();

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
	printf ("Board: Picochip " PICOCHIP_PLATFORM " \n");

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
 * Purpose: Initialize the DDR SDRAM size info.
 *
 * Returns: 0 - Success
 *
 *****************************************************************************/
int dram_init (void)
{
	gd->ram_size = get_ram_size ((long *)PHYS_SDRAM_1, PHYS_SDRAM_1_SIZE);

	return 0;
}

/*****************************************************************************
 *
 * dram_init_banksize()
 *
 * Purpose: Initialize the the bd_t dram bank info.
 *
 * Returns: 0 - Success
 *
 *****************************************************************************/
void dram_init_banksize (void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size =
	    get_ram_size ((long *)PHYS_SDRAM_1, PHYS_SDRAM_1_SIZE);
}
