/*****************************************************************************
 * $picoChipHeaderSubst$
 *****************************************************************************/

/*!
* \file pc73032.c
* \brief Various useful functions for use on a PC73032 Platform.
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
#include <asm/arch/axi2cfg.h>
#include <asm/arch/picoxcell_gpio.h>

/* Macros ------------------------------------------------------------------ */
static struct mux_def pc30xx_hnb_mux[] = {
	/*	Name		ARM	SD	PERIPH	REG	BIT	PERREG	PERBIT	FLAGS */
	MUXGPIO(usim_clk,	0,	16,	USIM,	0x34,	0,	0xc0,	4,	MUX_INVERT_PERIPH),
	MUXGPIO(usim_io,	1,	17,	USIM,	0x34,	1,	0xc0,	1,	MUX_INVERT_PERIPH),
	MUXGPIO(usim_vcc,	2,	18,	RSVD,	0x34,	2,	-1,	-1,	0),
	MUXGPIO(usim_rst,	3,	19,	RSVD,	0x34,	3,	-1,	-1,	0),
	MUXGPIO(usim_cd,	4,	20,	RSVD,	0x34,	4,	-1,	-1,	0),
	MUXGPIO(shd_gpio5,	5,	21,	RSVD,	0x34,	5,	-1,	-1,	0),
	MUXGPIO(shd_gpio6,	6,	22,	RSVD,	0x34,	6,	-1,	-1,	0),
	MUXGPIO(shd_gpio7,	7,	23,	RSVD,	0x34,	7,	-1,	-1,	0),
	MUXGPIO(shd_gpio8,	8,	8,	RSVD,	0x34,	8,	-1,	-1,	0),
	MUXGPIO(shd_gpio9,	9,	9,	RSVD,	0x34,	9,	-1,	-1,	0),
	MUXGPIO(shd_gpio10,	10,	10,	RSVD,	0x34,	10,	-1,	-1,	0),
	MUXGPIO(shd_gpio11,	11,	11,	RSVD,	0x34,	11,	-1,	-1,	0),
	MUXGPIO(shd_gpio12,	12,	12,	RSVD,	0x34,	12,	-1,	-1,	0),
	MUXGPIO(shd_gpio13,	13,	13,	RSVD,	0x34,	13,	-1,	-1,	0),
	MUXGPIO(shd_gpio14,	14,	14,	RSVD,	0x34,	14,	-1,	-1,	0),
	MUXGPIO(shd_gpio15,	15,	15,	FRACN,	0x34,	15,	0,	7,	MUX_INVERT_PERIPH),
	MUXGPIO(boot_mode0,	16,	0,	RSVD,	0x34,	16,	-1,	-1,	0),
	MUXGPIO(boot_mode1,	17,	1,	RSVD,	0x34,	17,	-1,	-1,	0),
	MUXGPIO(input_clk_sel0,	18,	2,	RSVD,	0x34,	18,	-1,	-1,	0),
	MUXGPIO(input_clk_sel1,	19,	3,	RSVD,	0x34,	19,	-1,	-1,	0),
	MUXGPIO(ssi_data_out,	22,	6,	SSI,	0x34,	22,	0x44,	0,	0),
	MUXGPIO(ssi_clk,	23,	7,	SSI,	0x34,	23,	0x44,	0,	0),
	MUXGPIO(ssi_data_in,	24,	-1,	SSI,	-1,	-1,	0x44,	0,	0),
	MUXGPIO(decode0,	25,	-1,	EBI,	-1,	-1,	0x40,	0,	0),
	MUXGPIO(decode1,	26,	-1,	EBI,	-1,	-1,	0x40,	1,	0),
	MUXGPIO(ebi_clk,	29,	-1,	EBI,	-1,	-1,	0x3c,	13,	0),
	MUXGPIO(pai_tx_data0,	47,	-1,	PAI,	-1,	-1,	0x38,	0,	0),
	MUXGPIO(pai_tx_data1,	48,	-1,	PAI,	-1,	-1,	0x38,	1,	0),
	MUXGPIO(pai_tx_data2,	49,	-1,	PAI,	-1,	-1,	0x38,	2,	0),
	MUXGPIO(pai_tx_data3,	50,	-1,	PAI,	-1,	-1,	0x38,	3,	0),
	MUXGPIO(pai_tx_data4,	51,	-1,	PAI,	-1,	-1,	0x38,	4,	0),
	MUXGPIO(pai_tx_data5,	52,	-1,	PAI,	-1,	-1,	0x38,	5,	0),
	MUXGPIO(pai_tx_data6,	53,	-1,	PAI,	-1,	-1,	0x38,	6,	0),
	MUXGPIO(pai_tx_data7,	54,	-1,	PAI,	-1,	-1,	0x38,	7,	0),
	MUXGPIO(pai_rx_data0,	55,	-1,	PAI,	-1,	-1,	0x38,	8,	0),
	MUXGPIO(pai_rx_data1,	56,	-1,	PAI,	-1,	-1,	0x38,	9,	0),
	MUXGPIO(pai_rx_data2,	57,	-1,	PAI,	-1,	-1,	0x38,	10,	0),
	MUXGPIO(pai_rx_data3,	58,	-1,	PAI,	-1,	-1,	0x38,	11,	0),
	MUXGPIO(pai_rx_data4,	59,	-1,	PAI,	-1,	-1,	0x38,	12,	0),
	MUXGPIO(pai_rx_data5,	60,	-1,	PAI,	-1,	-1,	0x38,	13,	0),
	MUXGPIO(pai_rx_data6,	61,	-1,	PAI,	-1,	-1,	0x38,	14,	0),
	MUXGPIO(pai_rx_data7,	62,	-1,	PAI,	-1,	-1,	0x38,	15,	0),

	/*	   Name			Periph	PeriphB	Reg	Bit */
	MUX2PERIPH(pad_pai_tx_clk,	PAI,	MAXIM,	0x4c,	0),
	MUX2PERIPH(pad_pai_tx_ctrl,	PAI,	MAXIM,	0x4c,	0),
	MUX2PERIPH(pad_pai_trig_clk,	PAI,	MAXIM,	0x4c,	0),
};

/* Constants --------------------------------------------------------------- */
DECLARE_GLOBAL_DATA_PTR;

/* Prototypes--------------------------------------------------------------- */
/*!
 * \brief Return the state of the reset switch
 *
 * \return 1 is the reset switch is pressed
 *         0 if the reset switch is not pressed
 */
static int is_reset_pressed (void);

/*!
 * \brief Perform factory reset action
 *
 * \return 0 if success
 *         not 0 for failure
 */
static int factory_reset_action (void);

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
	gd->bd->bi_arch_number = MACH_TYPE_PC73032;

	gd->flags = 0;

        /* Turn on some picoxcell block clocks */
        picoxcell_clk_enable (PICOXCELL_SSI_CLOCK);
        picoxcell_clk_enable (PICOXCELL_NAND_CLOCK);

	/* Start timer #0 */
	picoxcell_timer_start (0);

	/* Initialise the gpio muxing library */
	picoxcell_mux_register (pc30xx_hnb_mux, ARRAY_SIZE (pc30xx_hnb_mux));

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

static int is_reset_pressed (void)
{
	unsigned int reset = 0;

	reset =
	    picoxcell_read_register (PICOXCELL_GPIO_BASE +
				     GPIO_EXT_PORT_A_REG_OFFSET);
	reset &= CONFIG_SYS_RESET_PIN;

	return !reset;
}

static int factory_reset_action (void)
{
        /* This function needs to implement some as yet
         * undefined action
         */
        printf ("Factory reset requested...\n");

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
	unsigned int timebase = 0;
	int reset_held = 1;

	/* Check to see if the reset push button is being held down.
	 * If it is held down long enough we will be looking at a
	 * factory reset situation...
	 */
	if (is_reset_pressed ()) {

		timebase = get_timer (0);
		do {
			if (!is_reset_pressed ()) {
				reset_held = 0;
				break;
			}
		}
		while (get_timer (timebase) < CONFIG_SYS_RESET_DELAY);

		if (reset_held) {
			/* Factory reset requested */
                        factory_reset_action ();
                }
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

/*****************************************************************************
 *
 * pc30xx_check_memif_shared_setup()
 *
 * Purpose: Check to make sure the memif-shared is working correctly on
 *          PC30xx ES Si. See Mantis #9530 for full details.
 *
 *****************************************************************************/
void pc30xx_check_memif_shared_setup (void)
{
        unsigned int read_data;
        unsigned int write_data = 0x20120113;
        unsigned int address =
            is_memif_arm_usable_on_pc30xx () ? 0x10000000 : 0;

        picoxcell_write_register (write_data, address);
        read_data = picoxcell_read_register (address);
        if (read_data != write_data) {
                /* Oops, we have memory setup issues */
                reset_cpu (0);
        }
}
