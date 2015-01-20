/*****************************************************************************
 * $picoChipHeaderSubst$
 *****************************************************************************/

/*!
* \file pc7302.c
* \brief Various useful functions for use on a PC7302 Platform.
*
* Copyright (c) 2006-2012 Picochip Ltd
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
#include "ipa73xx_led.h"
#include "ipa73xx_fuse.h"


/* Macros ------------------------------------------------------------------ */
static struct mux_def pc3x2_mux[] = {
	/*	Name		ARM	SD	PERIPH	CAEID	CADDR	MASK*/
	MUXCFGBUS(sdgpio4,	-1,	4,	PAI,	0x8080,	0x9,	0x7),
	MUXCFGBUS(sdgpio5,	-1,	5,	PAI,	0x8080,	0x9,	0x6),
	MUXCFGBUS(sdgpio6,	-1,	6,	PAI,	0x8080,	0x9,	0x5),
	MUXCFGBUS(sdgpio7,	-1,	7,	PAI,	0x8080,	0x9,	0x4),

	MUXCFGBUS(arm4,		4,	-1,	PAI,	0x8080,	0x9,	0xb),
	MUXCFGBUS(arm5,		5,	-1,	PAI,	0x8080,	0x9,	0xa),
	MUXCFGBUS(arm6,		6,	-1,	PAI,	0x8080,	0x9,	0x9),
	MUXCFGBUS(arm7,		7,	-1,	PAI,	0x8080,	0x9,	0x8),

	/*	Name		ARM	SD	PERIPH	REG	BIT	PERREG	PERBIT	FLAGS */
	MUXGPIO(shared0,	8,	8,	FRACN,	0,	16,	0,	7,	0),
	MUXGPIO(shared1,	9,	9,	RSVD,	0,	17,	-1,	-1,	0),
	MUXGPIO(shared2,	10,	10,	RSVD,	0,	18,	-1,	-1,	0),
	MUXGPIO(shared3,	11,	11,	RSVD,	0,	19,	-1,	-1,	0),
	MUXGPIO(shared4,	12,	12,	RSVD,	0,	20,	-1,	-1,	0),
	MUXGPIO(shared5,	13,	13,	RSVD,	0,	21,	-1,	-1,	0),
	MUXGPIO(shared6,	14,	14,	RSVD,	0,	22,	-1,	-1,	0),
	MUXGPIO(shared7,	15,	15,	RSVD,	0,	23,	-1,	-1,	0),

	MUXGPIO(sdgpio0,	-1,	0,	FRACN,	-1,	-1,	0,	7,	MUX_INVERT_PERIPH),
};

static struct mux_def pc3x3_mux[] = {
	/*	Name		ARM	SD	PERIPH	REG	BIT	PERREG	PERBIT	FLAGS */
	MUXGPIO(arm_gpio0,	0,	16,	RSVD,	0x34,	0,	-1,	-1,	0),
	MUXGPIO(arm_gpio1,	1,	17,	RSVD,	0x34,	1,	-1,	-1,	0),
	MUXGPIO(arm_gpio2,	2,	18,	RSVD,	0x34,	2,	-1,	-1,	0),
	MUXGPIO(arm_gpio3,	3,	19,	RSVD,	0x34,	3,	-1,	-1,	0),
	MUXGPIO(shd_gpio,	8,	8,	RSVD,	0x34,	8,	-1,	-1,	0),
	MUXGPIO(boot_mode0,	9,	9,	RSVD,	0x34,	9,	-1,	-1,	0),
	MUXGPIO(boot_mode1,	10,	10,	RSVD,	0x34,	10,	-1,	-1,	0),
	MUXGPIO(sdram_speed_sel,11,	11,	RSVD,	0x34,	11,	-1,	-1,	0),
	MUXGPIO(mii_rev_en,	12,	12,	RSVD,	0x34,	12,	-1,	-1,	0),
	MUXGPIO(mii_rmii_en,	13,	13,	RSVD,	0x34,	13,	-1,	-1,	0),
	MUXGPIO(mii_speed_sel,	14,	14,	RSVD,	0x34,	14,	-1,	-1,	0),

	MUXGPIO(ebi_addr14,	32,	-1,	EBI,	-1,	-1,	0x3c,	0,	0),
	MUXGPIO(ebi_addr15,	33,	-1,	EBI,	-1,	-1,	0x3c,	1,	0),
	MUXGPIO(ebi_addr16,	34,	-1,	EBI,	-1,	-1,	0x3c,	2,	0),
	MUXGPIO(ebi_addr17,	35,	-1,	EBI,	-1,	-1,	0x3c,	3,	0),
	MUXGPIO(ebi_addr18,	20,	4,	EBI,	0x34,	20,	0x3c,	4,	0),
	MUXGPIO(ebi_addr19,	21,	5,	EBI,	0x34,	21,	0x3c,	5,	0),
	MUXGPIO(ebi_addr20,	22,	6,	EBI,	0x34,	22,	0x3c,	6,	0),
	MUXGPIO(ebi_addr21,	23,	7,	EBI,	0x34,	23,	0x3c,	7,	0),
	MUXGPIO(ebi_addr22,	4,	20,	EBI,	0x34,	4,	0x3c,	8,	0),
	MUXGPIO(ebi_addr23,	5,	21,	EBI,	0x34,	5,	0x3c,	9,	0),
	MUXGPIO(ebi_addr24,	6,	22,	EBI,	0x34,	6,	0x3c,	10,	0),
	MUXGPIO(ebi_addr25,	7,	23,	EBI,	0x34,	7,	0x3c,	11,	0),
	MUXGPIO(ebi_addr26,	15,	15,	EBI,	0x34,	15,	0x3c,	12,	0),
	MUXGPIO(ebi_clk_pin,	53,	-1,	EBI,	-1,	-1,	0x3c,	13,	0),

	MUXGPIO(pai_rx_data0,	20,	4,	PAI,	0x34,	20,	0x38,	8,	0),
	MUXGPIO(pai_rx_data1,	21,	5,	PAI,	0x34,	21,	0x38,	9,	0),
	MUXGPIO(pai_rx_data2,	22,	6,	PAI,	0x34,	22,	0x38,	10,	0),
	MUXGPIO(pai_rx_data3,	23,	7,	PAI,	0x34,	23,	0x38,	11,	0),
	MUXGPIO(pai_rx_data4,	28,	-1,	PAI,	-1,	-1,	0x38,	4,	0),
	MUXGPIO(pai_rx_data5,	29,	-1,	PAI,	-1,	-1,	0x38,	5,	0),
	MUXGPIO(pai_rx_data6,	30,	-1,	PAI,	-1,	-1,	0x38,	6,	0),
	MUXGPIO(pai_rx_data7,	31,	-1,	PAI,	-1,	-1,	0x38,	7,	0),

	MUXGPIO(pai_tx_data0,	4,	20,	PAI,	0x34,	4,	0x38,	0,	0),
	MUXGPIO(pai_tx_data1,	5,	21,	PAI,	0x34,	5,	0x38,	1,	0),
	MUXGPIO(pai_tx_data2,	6,	22,	PAI,	0x34,	6,	0x38,	2,	0),
	MUXGPIO(pai_tx_data3,	7,	23,	PAI,	0x34,	7,	0x38,	3,	0),
	MUXGPIO(pai_tx_data4,	24,	-1,	PAI,	-1,	-1,	0x38,	4,	0),
	MUXGPIO(pai_tx_data5,	25,	-1,	PAI,	-1,	-1,	0x38,	5,	0),
	MUXGPIO(pai_tx_data6,	26,	-1,	PAI,	-1,	-1,	0x38,	6,	0),
	MUXGPIO(pai_tx_data7,	27,	-1,	PAI,	-1,	-1,	0x38,	7,	0),

	MUXGPIO(decode0,	36,	-1,	DECODE,	-1,	-1,	0x40,	0,	0),
	MUXGPIO(decode1,	37,	-1,	DECODE,	-1,	-1,	0x40,	1,	0),
	MUXGPIO(decode2,	38,	-1,	DECODE,	-1,	-1,	0x40,	2,	0),
	MUXGPIO(decode3,	39,	-1,	DECODE,	-1,	-1,	0x40,	3,	0),

	MUXGPIO(ssi_clk,	40,	-1,	SSI,	-1,	-1,	0x44,	0,	0),
	MUXGPIO(ssi_data_in,	41,	-1,	SSI,	-1,	-1,	0x44,	0,	0),
	MUXGPIO(ssi_data_out,	42,	-1,	SSI,	-1,	-1,	0x44,	0,	0),

	MUXGPIO(mii_tx_data2,	43,	-1,	MII,	-1,	-1,	0,	13,	MUX_RO),
	MUXGPIO(mii_tx_data3,	44,	-1,	MII,	-1,	-1,	0,	13,	MUX_RO),
	MUXGPIO(mii_rx_data2,	45,	-1,	MII,	-1,	-1,	0,	13,	MUX_RO),
	MUXGPIO(mii_rx_data3,	46,	-1,	MII,	-1,	-1,	0,	13,	MUX_RO),
	MUXGPIO(mii_col,	47,	-1,	MII,	-1,	-1,	0,	13,	MUX_RO),
	MUXGPIO(mii_crs,	48,	-1,	MII,	-1,	-1,	0,	13,	MUX_RO),
	MUXGPIO(mii_tx_clk,	49,	-1,	MII,	-1,	-1,	0,	13,	MUX_RO),

	MUXGPIO(max_tx_ctrl,	50,	-1,	MAXIM,	-1,	-1,	0x44,	1,	0),
	MUXGPIO(max_ref_clk,	51,	-1,	MAXIM,	-1,	-1,	0x44,	1,	0),
	MUXGPIO(max_trig_clk,	52,	-1,	MAXIM,	-1,	-1,	0x44,	1,	0),

	MUXGPIO(sdgpio0,	-1,	0,	FRACN,	-1,	-1,	0,	7,	MUX_INVERT_PERIPH),
};

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
	gd->bd->bi_arch_number = MACH_TYPE_IPA73XX;

	gd->flags = 0;

	/* Start timer #0 */
	picoxcell_timer_start (0);

	/* Initialise the gpio muxing library */
	if (picoxcell_is_pc3x2 ()) {
		picoxcell_mux_register (pc3x2_mux, ARRAY_SIZE (pc3x2_mux));
	} else {
		picoxcell_mux_register (pc3x3_mux, ARRAY_SIZE (pc3x3_mux));
	}

	/* Initialise the gpio library */
	picoxcell_gpio_init ();

    led_confidence();

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
    char ethaddr_str[18];
    
    if (read_ethaddr_from_fuses(ethaddr_str, 0))
    {
        setenv("ethaddr", ethaddr_str);
        printf("ethaddr set from fuses: %s\n", ethaddr_str);
    }
    
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
	gd->ram_size = PHYS_SDRAM_1_SIZE;

	return 0;
}

/*****************************************************************************
 *
 * dram_init_banksize()
 *
 * Purpose: Initialize the the bd_t dram bank info.
 *
 *****************************************************************************/
void dram_init_banksize (void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;
}
