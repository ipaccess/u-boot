/*****************************************************************************
 * $picoChipHeaderSubst$
 *****************************************************************************/

/*
 * Copyright (c) 2010-2011 Picochip Ltd., Jamie Iles
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * All enquiries to support@picochip.com
 */

#include <linux/bitops.h>
#include <errno.h>
#include <asm/io.h>
#include <linux/string.h>

#include <asm/arch/mux.h>
#include <asm/arch/utilities.h>

static LIST_HEAD(mux_defs);

void picoxcell_mux_register(struct mux_def *defs, int nr_defs)
{
	int i;

	for (i = 0; i < nr_defs; ++i)
		list_add_tail(&defs[i].head, &mux_defs);
}

static int mux_set_config_bus(struct mux_def *def, enum mux_setting setting)
{
	u16 data = 0;

	if (setting != MUX_ARM && setting != MUX_SD)
		return -EINVAL;

	if ((setting == MUX_ARM && def->armgpio < 0) ||
	    (setting == MUX_SD && def->sdgpio < 0))
		return -EINVAL;

	/* Wake the AE up. */
	axi2cfg_config_write(def->caeid, 0xA060, &data, 1);

	/* Set the new muxing mask. */
	if (axi2cfg_config_read(def->caeid, def->caddr, &data, 1) != 1)
		return -EIO;
	data |= def->mask;
	axi2cfg_config_write(def->caeid, def->caddr, &data, 1);

	return 0;
}

static int mux_configure(struct mux_def *def, enum mux_setting setting)
{
	unsigned long periph_ctrl;

	if (def->flags & MUX_RO)
		return -EPERM;

	if (def->flags & MUX_CONFIG_BUS)
		return mux_set_config_bus(def, setting);

	if (!((def->armgpio >= 0 && setting == MUX_ARM) ||
	      (def->sdgpio >= 0 && setting == MUX_SD) ||
	      (def->periph >= 0 && setting == def->periph)))
		return -EINVAL;

	if (def->periph > 0) {
		periph_ctrl = axi2cfg_readl(def->periph_reg);

		if (setting == def->periph) {
			/* Enable the peripheral. */
			if (def->flags & MUX_INVERT_PERIPH)
				periph_ctrl |= (1 << def->periph_bit);
			else
				periph_ctrl &= ~(1 << def->periph_bit);
		} else {
			/* Disable the peripheral. */
			if (def->flags & MUX_INVERT_PERIPH)
				periph_ctrl &= ~(1 << def->periph_bit);
			else
				periph_ctrl |= (1 << def->periph_bit);
		}
		axi2cfg_writel(periph_ctrl, def->periph_reg);

		if (def->periph_b >= 0 && setting == def->periph_b)
			return 0;
	}

	if (setting != def->periph && def->gpio_reg_offs >= 0) {
		unsigned long gpio_sel = axi2cfg_readl(def->gpio_reg_offs);

		if (setting == MUX_SD)
			gpio_sel &= ~(1 << def->gpio_reg_bit);
		else
			gpio_sel |= (1 << def->gpio_reg_bit);

		axi2cfg_writel(gpio_sel, def->gpio_reg_offs);
	}

	return 0;
}

int mux_configure_one(const char *name, enum mux_setting setting)
{
	struct mux_def *def = NULL;

	list_for_each_entry(def, &mux_defs, head)
		if (!strcmp(name, def->name))
			return mux_configure(def, setting);

	return -ENXIO;
}

int mux_configure_table(const struct mux_cfg *cfg,
			unsigned int nr_cfgs)
{
	unsigned int n;
	int ret = 0;

	for (n = 0; n < nr_cfgs; ++n) {
		ret = mux_configure_one(cfg[n].name, cfg[n].setting);
		if (ret)
			break;
	}

	return ret;
}
