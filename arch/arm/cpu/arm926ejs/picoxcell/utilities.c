/*****************************************************************************
 * $picoChipHeaderSubst$
 *****************************************************************************/

/*!
 * \file utilities.c
 * \brief Various useful functions.
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
/*
 * \brief Is the memif arm present on a PC30xx ?
 *
 * \return 1 if memif arm present
 *         0 if memif arm not present
 */
static int is_memif_arm_present_on_pc30xx (void);

/*!
 * \brief Is the memif arm interface disabled ?
 *
 * \return 1 if memif arm is disabled
 *         0 if memif arm is not disabled
 */
static int is_memif_arm_disabled (void);

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

const char *picoxcell_get_partname (void)
{
	unsigned long dev_id = axi2cfg_readl (AXI2CFG_DEVICE_ID_REG_OFFSET);
	const char *part = "<unknown>";

	if (dev_id == 0x8003)
		part = "pc302";
	else if (dev_id == 0x8007)
		part = "pc312";
	else if (dev_id == 0x20)
		part = "pc313";
	else if (dev_id == 0x21)
		part = "pc323";
	else if (dev_id == 0x22)
		part = "pc333";
	else if (dev_id == 0x30)
		part = "pc3008";
	else if (dev_id == 0x31)
		part = "pc3032";
	return part;
}

unsigned int picoxcell_get_revision (void)
{
	unsigned int revision_code;

	revision_code = axi2cfg_readl (AXI2CFG_REVISION_ID_REG_OFFSET);
	revision_code &= SIXTEEN_BIT_MASK;

	return revision_code;
}

unsigned int picoxcell_is_rmii_enabled (void)
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

int picoxcell_is_pc3x2 (void)
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

int picoxcell_is_pc3x3 (void)
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

int picoxcell_is_pc30xx (void)
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

void picoxcell_clk_enable (unsigned long clock)
{
    unsigned long clk_gate;

    if (picoxcell_is_pc30xx ())
    {
        clk_gate = axi2cfg_readl (AXI2CFG_CLK_GATING_REG_OFFSET);

        /* Block clocks are enabled by disabling the clock gating */
        clk_gate &= ~(clock);
        axi2cfg_writel (clk_gate, AXI2CFG_CLK_GATING_REG_OFFSET);
    }
}

static int is_memif_arm_present_on_pc30xx (void)
{
	unsigned int memif_present;

	memif_present = picoxcell_read_register (PICOXCELL_AXI2CFG_BASE +
					         AXI2CFG_ID_REG_OFFSET);
	memif_present &= PICOXCELL_MEMIF_ARM_NOT_PRESENT;

	return !memif_present;
}

static int is_memif_arm_disabled (void)
{
	unsigned int memif_disabled;

	memif_disabled = picoxcell_read_register (PICOXCELL_FUSE_BASE +
					          FUSE_MAP_31_REG_OFFSET);
	memif_disabled &= PICOXCELL_DISABLE_MEMIF_ARM;

	return ! !memif_disabled;
}

int is_memif_arm_usable_on_pc30xx (void)
{
	if (is_memif_arm_present_on_pc30xx () && !is_memif_arm_disabled ()) {
		return 1;
	} else {
		return 0;
	}
}
