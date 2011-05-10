/*****************************************************************************
 * $picoChipHeaderSubst$
 *****************************************************************************/

/*!
 * \file utilities.c
 * \brief Various useful functions.
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
#include <asm/arch/fuse.h>
#include <asm/arch/timer.h>
#include <asm/arch/utilities.h>

/* Macros ------------------------------------------------------------------ */
/*!
 * Bit mask used to obtain the least significant 16 bits
 */
#define SIXTEEN_BIT_MASK        ( 0xFFFF )

/*!
 * Bit mask used to obtain the device ID
 */
#define DEVICE_ID_MASK          ( 0xFF )

/* Constants --------------------------------------------------------------- */

/* Types ------------------------------------------------------------------- */

/* Prototypes--------------------------------------------------------------- */

/* Functions --------------------------------------------------------------- */
__inline unsigned int picoxcell_read_register (const unsigned int address)
{
	/* Read an integer (32 bit) value from a register */
	return (*(volatile unsigned int *)address);
}

__inline void
picoxcell_write_register (const unsigned int value, const unsigned int address)
{
	/* Write an integer (32 bit) value to a register */
	*(volatile unsigned int *)address = value;
}

unsigned int picoxcell_get_device_id (void)
{
	unsigned int device_id;

	device_id = axi2cfg_readl (AXI2CFG_DEVICE_ID_REG_OFFSET);
	device_id &= DEVICE_ID_MASK;

	return device_id;
}

unsigned int picoxcell_get_revision (void)
{
	unsigned int revision_code;

	revision_code = axi2cfg_readl (AXI2CFG_REVISION_ID_REG_OFFSET);
	revision_code &= SIXTEEN_BIT_MASK;

	return revision_code;
}

unsigned int picoxcell_get_rmii_enabled (void)
{
	unsigned int rmii_enabled;

	rmii_enabled = axi2cfg_readl (AXI2CFG_SYS_CONFIG_REG_OFFSET);
	rmii_enabled &= AXI2CFG_RMII_EN;

	return ! !rmii_enabled;
}

u32 syscfg_read (void)
{
	return axi2cfg_readl (AXI2CFG_SYS_CONFIG_REG_OFFSET);
}

void syscfg_update (u32 mask, u32 val)
{
	u32 tmp = syscfg_read ();
	tmp &= ~mask;
	tmp |= (val & mask);
	axi2cfg_writel (tmp, AXI2CFG_SYS_CONFIG_REG_OFFSET);
}

int picoxcell_is_pc3x2(void)
{
        unsigned long device_id = axi2cfg_readl (AXI2CFG_DEVICE_ID_REG_OFFSET);

        switch (device_id) {
        case 0x8003:
        case 0x8007:
                return 1;
        default:
                return 0;
        }
}

int picoxcell_is_pc3x3(void)
{
        unsigned long device_id = axi2cfg_readl (AXI2CFG_DEVICE_ID_REG_OFFSET);

        switch (device_id) {
        case 0x20:
        case 0x21:
        case 0x22:
                return 1;
        default:
                return 0;
        }
}

int picoxcell_is_pc30xx(void)
{
        unsigned long device_id = axi2cfg_readl (AXI2CFG_DEVICE_ID_REG_OFFSET);
        switch (device_id) {
        case 0x30 ... 0x3F:
                return 1;
        default:
                return 0;
        }
}

void picoxcell_timer_start (int timer)
{
	/* Make sure the timer is disabled */
	picoxcell_write_register (0, (CONFIG_SYS_TIMERBASE +
				      TIMERNCONTROLREGOFFSET (timer)));

	/* Initialise the timer to all 1's.
	 * We do this  because we want to run
	 * the timer in free running mode.
	 */
	picoxcell_write_register (0xFFFFFFFF, (CONFIG_SYS_TIMERBASE +
					       TIMERNLOADCOUNTREGOFFSET
					       (timer)));

	/* Start the timer in free running mode */
	picoxcell_write_register ((TIMERINTERRUPTMASK | TIMERENABLE),
				  (CONFIG_SYS_TIMERBASE +
				   TIMERNCONTROLREGOFFSET (timer)));

}
