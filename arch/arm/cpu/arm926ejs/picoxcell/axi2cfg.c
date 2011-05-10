/*****************************************************************************
 * $picoChipHeaderSubst$
 *****************************************************************************/

/*!
 * \file axi2cfg.c
 * \brief Various useful functions for the axi2cfg block.
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
#include <asm/errno.h>
#include <asm/io.h>
#include <asm/arch/picoxcell.h>
#include <asm/arch/axi2cfg.h>
#include <asm/arch/utilities.h>

/* Macros ------------------------------------------------------------------ */
/* Configuration port write bit positions. */
#define CAEID_BIT_MASK     ( 1 << 19 )	/*!< Bit 19 - AE ID signal. */
#define CADDR_BIT_MASK     ( 1 << 18 )	/*!< Bit 18 - AE ADDR signal. */
#define CREAD_BIT_MASK     ( 1 << 17 )	/*!< Bit 17 - READ data signal. */
#define CWRITE_BIT_MASK    ( 1 << 16 )	/*!< Bit 16 - WRITE data signal. */

#define RB_FAIL_MASK       ( 1 << 17 )	/*!< Bit 17 - readback failed. */
#define RB_VALID_MASK      ( 1 << 16 )	/*!< Bit 16 - readback valid. */

#define RETRIES ( 10 )		/*!< The number of retries for an \
				 *   AXI2Cfg config read. */

/*! Register offset for the config bus write port (from the axi2cfg2 base
 *  address). */
#define AXI2CFG_REG_CFG_WR ( 0x0100 )

/*! Register offset for the config bus read port (from the axi2cfg2 base
 *  address). */
#define AXI2CFG_REG_CFG_RD ( 0x0200 )

/* Constants --------------------------------------------------------------- */

/* Types ------------------------------------------------------------------- */

/* Prototypes--------------------------------------------------------------- */

/* Functions --------------------------------------------------------------- */
unsigned long axi2cfg_readl(unsigned long offs)
{
        return readl(PICOXCELL_AXI2CFG_BASE + offs);
}

void axi2cfg_writel(unsigned long val, unsigned long offs)
{
        writel(val, (PICOXCELL_AXI2CFG_BASE + offs));
}

int axi2cfg_config_read (u16 caeid, u16 address, u16 * data, u16 count)
{
	u32 val;
	unsigned int write_p = PICOXCELL_AXI2CFG_BASE + AXI2CFG_REG_CFG_WR;
	unsigned int read_p = PICOXCELL_AXI2CFG_BASE + AXI2CFG_REG_CFG_RD;
	u16 to_read = count;
	u16 rc;
	unsigned i;
	unsigned retries;

	val = caeid | CAEID_BIT_MASK;
	picoxcell_write_register (val, write_p);

	while (to_read) {
		/* Output the address to read from. */
		val = (address + (count - to_read)) | CADDR_BIT_MASK;
		picoxcell_write_register (val, write_p);

		/* Dispatch the read requests. */
		rc = (to_read > 64) ? 64 : to_read;
		val = CREAD_BIT_MASK | rc;
		picoxcell_write_register (val, write_p);

		/* Now read the values. */
		for (i = 0; i < rc; ++i) {
			retries = RETRIES;
			while (retries) {
				val = picoxcell_read_register (read_p);
				if (val & (RB_VALID_MASK | RB_FAIL_MASK))
					break;
				--retries;
			}

			if (!retries || (val & RB_FAIL_MASK)) {
				break;
			} else
				data[(count - to_read) + i] = val & 0xFFFF;
		}

		if (val & RB_FAIL_MASK)
			break;

		to_read -= rc;
	}

	return !(val & RB_FAIL_MASK) ? count : -EIO;
}

int axi2cfg_config_write (u16 caeid, u16 address, u16 * data, u16 count)
{
	u32 val;
	unsigned int write_p = PICOXCELL_AXI2CFG_BASE + AXI2CFG_REG_CFG_WR;
	unsigned i;

	val = caeid | CAEID_BIT_MASK;
	picoxcell_write_register (val, write_p);

	/* Output the address to write to */
	val = address | CADDR_BIT_MASK;
	picoxcell_write_register (val, write_p);

	/* Now write the values. */
	for (i = 0; i < count; ++i) {
		val = data[i] | CWRITE_BIT_MASK;
		picoxcell_write_register (val, write_p);
	}

	return i;
}
