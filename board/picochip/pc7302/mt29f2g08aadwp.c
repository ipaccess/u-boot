/*****************************************************************************
 * $picoChipHeaderSubst$
 *****************************************************************************/

/*!
* \file .mt29f2g08aadwp.c
* \brief Support for the NAND Flash device fitted on PC7302 platform.
*
* Copyright (c) 2009-2011 Picochip Ltd
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
* All enquiries to support@picochip.com
*/

/*
 * (C) Copyright 2009 SAGEM Communications
 * (C) Copyright 2006 DENX Software Engineering
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */



/* Includes ---------------------------------------------------------------- */
#include <common.h>

#ifdef CONFIG_CMD_NAND

#include <asm/arch/pc302.h>
#include <nand.h>
#include <asm/arch/utilities.h>
#include <asm/arch/pc3xxgpio.h>
#include <asm/arch/mux.h>
#include <asm/arch/axi2cfg.h>

/* Constants --------------------------------------------------------------- */
static unsigned int cle;
static unsigned int ale;
static unsigned int nce;
static unsigned int rdy;

/* Functions --------------------------------------------------------------- */
/*!
 * \brief Hardware specific access to control-lines
 * \param mtd, pointer to the mtd_info structure
 * \param dat, data to write to the device
 * \param ctrl, control data to set up the transaction
 *
 */
static void mt29f2g08aadwp_cmd_ctrl(struct mtd_info *mtd,
				    int dat,
				    unsigned int ctrl)
{
    struct nand_chip *this = mtd->priv;

    if (ctrl & NAND_CTRL_CHANGE)
    {
        if (ctrl & NAND_NCE)
        {
            /* Assert the chip select */
            pc3xx_gpio_set_value(nce, 0);

	    if (ctrl & NAND_CLE)
            {
	        /* Assert CLE */
                pc3xx_gpio_set_value(cle, 1);
            }
    	    else
	    {
                /* Negate CLE */
                pc3xx_gpio_set_value(cle, 0);
            }

	    if (ctrl & NAND_ALE)
	    {
                /* Assert ALE */
                pc3xx_gpio_set_value(ale, 1);
            }
	    else
            {
		/* Negate ALE */
                pc3xx_gpio_set_value(ale, 0);
            }
        }
        else
        {
	    /* Negate the chip select */
            pc3xx_gpio_set_value(nce, 1);
        }
    }

    /* If we have data to write, write it */
    if (dat != NAND_CMD_NONE)
    {
	*(volatile unsigned char *)(this->IO_ADDR_W) = (unsigned char)dat;
    }
}

/*!
 * \brief Return the state of the NAND busy output
 * \param mtd, pointer to the mtd_info structure
 * \return 0 - nand busy
 *         1 - nand ready
 *
 */
static int mt29f2g08aadwp_dev_ready(struct mtd_info *mtd)
{
    return pc3xx_gpio_get_value(rdy);
}

/*
 * Board-specific NAND initialization. The following members of the
 * argument are board-specific (per include/linux/mtd/nand.h):
 * - IO_ADDR_R?: address to read the 8 I/O lines of the flash device
 * - IO_ADDR_W?: address to write the 8 I/O lines of the flash device
 * - hwcontrol: hardwarespecific function for accesing control-lines
 * - dev_ready: hardwarespecific function for  accesing device ready/busy line
 * - enable_hwecc?: function to enable (reset)  hardware ecc generator. Must
 *   only be provided if a hardware ECC is available
 * - eccmode: mode of ecc, see defines
 * - chip_delay: chip dependent delay for transfering data from array to
 *   read regs (tR)
 * - options: various chip options. They can partly be set to inform
 *   nand_scan about special functionality. See the defines for further
 *   explanation
 * Members with a "?" were not set in the merged testing-NAND branch,
 * so they are not set here either.
 */
int board_nand_init(struct nand_chip *nand)
{
    /* Define which gpio bits are used to control the NAND Flash
     *
     * Note: These pin definitions mean that we can only use NAND
     *       Flash if we are running U-Boot from RAM and have NOT booted
     *       the device from parallel NOR Flash.
     */
    if (is_pc3x3())
    {
        cle = PC3X3_GPIO_PIN_ARM_4;
        ale = PC3X3_GPIO_PIN_ARM_3;
        nce = PC3X3_GPIO_PIN_ARM_2;
        rdy = PC3X3_GPIO_PIN_ARM_1;

        /* Setup some pin muxing */
        pc3xx_group_set_mux ("pai_tx_data[3:0]", MUX_PERIPHERAL);
        pc3xx_pin_set_mux(PC3X3_GPIO_PIN_ARM_4, MUX_ARM);
    }
    else
    {
        cle = PC302_GPIO_PIN_ARM_4;
        ale = PC302_GPIO_PIN_ARM_3;
        nce = PC302_GPIO_PIN_ARM_2;
        rdy = PC302_GPIO_PIN_ARM_1;

        /* Setup some pin muxing */
        pc3xx_pin_set_mux(PC302_GPIO_PIN_ARM_4, MUX_ARM);
    }

    /* Request the required gpio pins */
    (void)pc3xx_gpio_request(rdy);
    (void)pc3xx_gpio_request(nce);
    (void)pc3xx_gpio_request(ale);
    (void)pc3xx_gpio_request(cle);

    /* Initialise the pin direction */
    (void)pc3xx_gpio_direction_input(rdy );
    (void)pc3xx_gpio_direction_output(nce, 1);
    (void)pc3xx_gpio_direction_output(ale, 0);
    (void)pc3xx_gpio_direction_output(cle, 0);

    /* Populate some members of the nand structure */
    nand->cmd_ctrl = mt29f2g08aadwp_cmd_ctrl;
    nand->ecc.mode = NAND_ECC_SOFT;
    nand->dev_ready = mt29f2g08aadwp_dev_ready;
    nand->IO_ADDR_R = (void __iomem *)CONFIG_SYS_NAND_BASE;
    nand->IO_ADDR_W = (void __iomem *)CONFIG_SYS_NAND_BASE;

    return 0;
}
#endif /* CONFIG_CMD_NAND */
