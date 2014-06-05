/*****************************************************************************
 * $picoChipHeaderSubst$
 *****************************************************************************/

/*!
 * \file mux.c
 * \brief Muxing control for the gpio pins.
 *
 * Copyright (c) 2009-2011 Picochip Ltd.
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
#include <asm/arch/pc302.h>
#include <asm/arch/utilities.h>
#include <asm/arch/axi2cfg.h>
#include <asm/arch/mux.h>
#include <asm/arch/pc3xxgpio.h>

/* Constants --------------------------------------------------------------- */
/*
 * A multiplexed pin. This defines the SD and ARM pins that are on the pad. If
 * the pin does not have an SD or ARM pin then set the appropriate field to
 * -1.
 */
struct muxed_pin {
	const char	*name;
	int		is_dedicated_gpio;
	int		sd_pin;
	int		arm_pin;
	int		(*set_mux)(struct muxed_pin *pin,
			           enum mux_setting setting);
	int		(*get_mux)(struct muxed_pin *pin);
};

/*
 * A logical group of multiplexed pins. Typically this is grouped by what the
 * pins are multiplexed with e.g. system peripheral.
 */
struct pin_group {
	int		    nr_pins;
	const char	    *name;
	struct muxed_pin    *pins;
};

/* Macros ------------------------------------------------------------------ */
#define __PIN(_name, _sd, _arm, _set, _get)		        \
	.name		    = __stringify(_name),		\
	.is_dedicated_gpio  = 0,				\
	.sd_pin		    = (_sd),				\
	.arm_pin    	    = (_arm),				\
	.set_mux    	    = _set,				\
	.get_mux    	    = _get
/*
 * Declare a function pin that is also multiplexed with GPIO pins.
 */
#define PIN(_name, _sd, _arm, _set, _get) {			\
	__PIN(_name, _sd, _arm, _set, _get),			\
	.is_dedicated_gpio  = 0,				\
}

/*
 * Declare a pure GPIO pin.
 */
#define GPIO(_name, _sd, _arm, _set, _get) {			\
	__PIN(_name, _sd, _arm, _set, _get),			\
	.is_dedicated_gpio  = 1,				\
}

/* Functions --------------------------------------------------------------- */
/*****************************************************************************
 * PC302 pin multiplexing.
 ****************************************************************************/
#ifdef CONFIG_PICOCHIP_PC3X2

static int
pai_get_mux(struct muxed_pin *pin)
{
#define PAI_GPIO_PIN_ARM_4	0xB
#define PAI_GPIO_PIN_ARM_5	0xA
#define PAI_GPIO_PIN_ARM_6	0x9
#define PAI_GPIO_PIN_ARM_7	0x8
#define PAI_GPIO_PIN_SDGPIO_4	0x7
#define PAI_GPIO_PIN_SDGPIO_5	0x6
#define PAI_GPIO_PIN_SDGPIO_6	0x5
#define PAI_GPIO_PIN_SDGPIO_7	0x4
#define PC302_PAI_CAEID		0x8080
#define PAI_SLEEP_REG		0xA060
#define PAI_IO_CTRL_REG		0x0009

	/* Make sure that the PAI block is awake. */
	u16 data = 0, sd_mask = 0, arm_mask = 0;
	int ret = axi2cfg_config_write(PC302_PAI_CAEID, PAI_SLEEP_REG,
				       &data, 1);
	if (1 != ret) {
		printf("unable to wake up PAI\n");
		return -EIO;
	}

	/* Get the current PAI muxing configuration. */
	ret = axi2cfg_config_read(PC302_PAI_CAEID, PAI_IO_CTRL_REG,
				  &data, 1);
	if (1 != ret) {
		printf("unable to read PAI I/O control reg\n");
		return -EIO;
	}

	if (pin->sd_pin == PC302_GPIO_PIN_SDGPIO_4) {
		sd_mask |= PAI_GPIO_PIN_SDGPIO_4;
	} else if (pin->sd_pin == PC302_GPIO_PIN_SDGPIO_5) {
		sd_mask |= PAI_GPIO_PIN_SDGPIO_5;
	} else if (pin->sd_pin == PC302_GPIO_PIN_SDGPIO_6) {
		sd_mask |= PAI_GPIO_PIN_SDGPIO_6;
	} else if (pin->sd_pin == PC302_GPIO_PIN_SDGPIO_7) {
		sd_mask |= PAI_GPIO_PIN_SDGPIO_7;
	} else if (pin->arm_pin == PC302_GPIO_PIN_ARM_4) {
		arm_mask |= PAI_GPIO_PIN_ARM_4;
	} else if (pin->arm_pin == PC302_GPIO_PIN_ARM_5) {
		arm_mask |= PAI_GPIO_PIN_ARM_5;
	} else if (pin->arm_pin == PC302_GPIO_PIN_ARM_6) {
		arm_mask |= PAI_GPIO_PIN_ARM_6;
	} else if (pin->arm_pin == PC302_GPIO_PIN_ARM_7) {
		arm_mask |= PAI_GPIO_PIN_ARM_7;
	} else {
		ret = -EINVAL;
		goto out;
	}

	if (data & sd_mask)
		ret = MUX_SD;
	else if (data & arm_mask)
		ret = MUX_ARM;
	else
		ret = MUX_PERIPHERAL;

out:
	return ret;
}

static int
pai_set_mux(struct muxed_pin *pin,
	    enum mux_setting setting)
{
	/* Make sure that the PAI block is awake. */
	u16 data = 0;
	int err = axi2cfg_config_write(PC302_PAI_CAEID, PAI_SLEEP_REG,
				       &data, 1);
	if (1 != err) {
		printf("unable to wake up PAI\n");
		return -EIO;
	}

	/* Get the current PAI muxing configuration. */
	err = axi2cfg_config_read(PC302_PAI_CAEID, PAI_IO_CTRL_REG,
				  &data, 1);
	if (1 != err) {
		printf("unable to read PAI I/O control reg\n");
		return -EIO;
	}

	if (MUX_SD == setting) {
		if (pin->sd_pin == PC302_GPIO_PIN_SDGPIO_4) {
			data |= PAI_GPIO_PIN_SDGPIO_4;
		} else if (pin->sd_pin == PC302_GPIO_PIN_SDGPIO_5) {
			data |= PAI_GPIO_PIN_SDGPIO_5;
		} else if (pin->sd_pin == PC302_GPIO_PIN_SDGPIO_6) {
			data |= PAI_GPIO_PIN_SDGPIO_6;
		} else if (pin->sd_pin == PC302_GPIO_PIN_SDGPIO_7) {
			data |= PAI_GPIO_PIN_SDGPIO_7;
		} else {
			err = -EINVAL;
			goto out;
		}
	} else if (MUX_ARM == setting) {
		if (pin->arm_pin == PC302_GPIO_PIN_ARM_4) {
			data |= PAI_GPIO_PIN_ARM_4;
		} else if (pin->arm_pin == PC302_GPIO_PIN_ARM_5) {
			data |= PAI_GPIO_PIN_ARM_5;
		} else if (pin->arm_pin == PC302_GPIO_PIN_ARM_6) {
			data |= PAI_GPIO_PIN_ARM_6;
		} else if (pin->arm_pin == PC302_GPIO_PIN_ARM_7) {
			data |= PAI_GPIO_PIN_ARM_7;
		} else {
			err = -EINVAL;
			goto out;
		}
	} else if (MUX_PERIPHERAL == setting) {
		/*
		 * We don't do anything here. The PAI is only driven by the
		 * picoArray so this will be automatically set in the loadfile
		 * when the design is next loaded.
		 */
		err = 0;
	} else if (MUX_UNMUXED == setting) {
		/* This can't be changed at run-time! */
		return -EPERM;
	}

	err = axi2cfg_config_write(PC302_PAI_CAEID, PAI_IO_CTRL_REG, &data, 1);
	if (1 != err) {
		printf("unable to write PAI I/O control reg\n");
		return -EIO;
	}

	err = 0;

out:
	return err;
}

static struct muxed_pin pai_pins[] = {
PIN(sdgpio4, PC302_GPIO_PIN_SDGPIO_4, -1, pai_set_mux, pai_get_mux),
PIN(sdgpio5, PC302_GPIO_PIN_SDGPIO_5, -1, pai_set_mux, pai_get_mux),
PIN(sdgpio6, PC302_GPIO_PIN_SDGPIO_6, -1, pai_set_mux, pai_get_mux),
PIN(sdgpio7, PC302_GPIO_PIN_SDGPIO_7, -1, pai_set_mux, pai_get_mux),
PIN(arm4, -1, PC302_GPIO_PIN_ARM_4, pai_set_mux, pai_get_mux),
PIN(arm5, -1, PC302_GPIO_PIN_ARM_5, pai_set_mux, pai_get_mux),
PIN(arm6, -1, PC302_GPIO_PIN_ARM_6, pai_set_mux, pai_get_mux),
PIN(arm7, -1, PC302_GPIO_PIN_ARM_7, pai_set_mux, pai_get_mux),
};

static struct pin_group pai_group = {
	.nr_pins    = ARRAY_SIZE(pai_pins),
	.name	    = "pai/ebi",
	.pins	    = pai_pins,
};

static int
shd_get_mux(struct muxed_pin *pin)
{
	unsigned bit;
	unsigned long syscfg = syscfg_read();

	if (0 == (syscfg & 0x3) && pin->sd_pin == PC302_GPIO_PIN_SDGPIO_15)
		return MUX_PERIPHERAL;

	if (pin->sd_pin == PC302_GPIO_PIN_SDGPIO_0) {
		if (syscfg & AXI2CFG_SYS_CONFIG_FREQ_SYNTH_MUX_MASK)
			return MUX_PERIPHERAL;
		else
			return MUX_SD;
	}

	bit = 1 << (AXI2CFG_SYS_CONFIG_SD_ARM_GPIO_SEL_LO +
			pin->sd_pin - PC302_GPIO_PIN_SDGPIO_8);

	return syscfg & bit ? MUX_ARM : MUX_SD;
}

static int
shd_set_mux(struct muxed_pin *pin,
	    enum mux_setting setting)
{
	unsigned bit;
	unsigned long syscfg = syscfg_read();

	/*
	 * In parallel boot mode, shared pin 7 can't be used as it is always
	 * on the EBI.
	 */
	if (0 == (syscfg & 0x3) && pin->sd_pin == PC302_GPIO_PIN_SDGPIO_15)
		return -EBUSY;

	switch (setting) {
	case MUX_PERIPHERAL:
		/*
		 * SDGPIO pin 0 is shared with the Frac-N.
		 */
		if (PC302_GPIO_PIN_SDGPIO_0 == pin->sd_pin) {
			syscfg_update(AXI2CFG_SYS_CONFIG_FREQ_SYNTH_MUX_MASK,
				      AXI2CFG_SYS_CONFIG_FREQ_SYNTH_MUX_MASK);
			break;
		}
		return -EIO;

	case MUX_SD:
		syscfg_update(AXI2CFG_SYS_CONFIG_FREQ_SYNTH_MUX_MASK, 0);
		bit = 1 << (AXI2CFG_SYS_CONFIG_SD_ARM_GPIO_SEL_LO +
			    pin->sd_pin - PC302_GPIO_PIN_SDGPIO_8);
		syscfg_update(bit, 0);
		break;

	case MUX_ARM:
		if (PC302_GPIO_PIN_SDGPIO_0 == pin->sd_pin)
			return -EINVAL;
		syscfg_update(AXI2CFG_SYS_CONFIG_FREQ_SYNTH_MUX_MASK, 0);
		bit = 1 << (AXI2CFG_SYS_CONFIG_SD_ARM_GPIO_SEL_LO +
			    pin->arm_pin - PC302_GPIO_PIN_ARM_8);
		syscfg_update(bit, bit);
		break;

	case MUX_UNMUXED:
		/* This can't be changed at run-time! */
		return -EPERM;
	}

	return 0;
}

static struct muxed_pin shared_pins[] = {
GPIO(shared0, PC302_GPIO_PIN_SDGPIO_8, PC302_GPIO_PIN_ARM_8, shd_set_mux,
     shd_get_mux),
GPIO(shared1, PC302_GPIO_PIN_SDGPIO_9, PC302_GPIO_PIN_ARM_9, shd_set_mux,
     shd_get_mux),
GPIO(shared2, PC302_GPIO_PIN_SDGPIO_10, PC302_GPIO_PIN_ARM_10, shd_set_mux,
     shd_get_mux),
GPIO(shared3, PC302_GPIO_PIN_SDGPIO_11, PC302_GPIO_PIN_ARM_11, shd_set_mux,
     shd_get_mux),
GPIO(shared4, PC302_GPIO_PIN_SDGPIO_12, PC302_GPIO_PIN_ARM_12, shd_set_mux,
     shd_get_mux),
GPIO(shared5, PC302_GPIO_PIN_SDGPIO_13, PC302_GPIO_PIN_ARM_13, shd_set_mux,
     shd_get_mux),
GPIO(shared6, PC302_GPIO_PIN_SDGPIO_14, PC302_GPIO_PIN_ARM_14, shd_set_mux,
     shd_get_mux),
PIN(shared7, PC302_GPIO_PIN_SDGPIO_15, PC302_GPIO_PIN_ARM_15, shd_set_mux,
    shd_get_mux),
};

static struct pin_group shd_group = {
	.nr_pins    = ARRAY_SIZE(shared_pins),
	.name	    = "shared/ebi",
	.pins	    = shared_pins,
};

static struct muxed_pin fracn_pins[] = {
PIN(sdgpio0, PC302_GPIO_PIN_SDGPIO_0, -1, shd_set_mux, shd_get_mux),
};

static struct pin_group fracn_group = {
	.nr_pins    = ARRAY_SIZE(fracn_pins),
	.name	    = "fracn/sdgpio0",
	.pins	    = fracn_pins,
};

static struct pin_group *pc3x2_groups[] = {
	&shd_group,
	&pai_group,
	&fracn_group,
};
#endif /* CONFIG_PICOCHIP_PC3X2 */

/*****************************************************************************
 * PC3X3 pin multiplexing.
 ****************************************************************************/
#ifdef CONFIG_PICOCHIP_PC3X3

#define SHD_GPIO_MUX_REG        (PC302_AXI2CFG_BASE + \
				 AXI2CFG_SHD_GPIO_MUXING_REG_OFFSET)
#define USE_EBI_GPIO_REG	(PC302_AXI2CFG_BASE + \
				 AXI2CFG_USE_EBI_GPIO_REG_OFFSET)
#define USE_PAI_GPIO_REG	(PC302_AXI2CFG_BASE + \
				 AXI2CFG_USE_PAI_GPIO_REG_OFFSET)
#define USE_DECODE_GPIO_REG	(PC302_AXI2CFG_BASE + \
				 AXI2CFG_USE_DECODE_GPIO_REG_OFFSET)
#define USE_MISC_INT_GPIO_REG	(PC302_AXI2CFG_BASE + \
				 AXI2CFG_USE_MISC_INT_GPIO_REG_OFFSET)

/*
 * Set the muxing of one of the shared pins.
 */
static void
pc3xx_shd_gpio_set_mux(int arm_pin_nr,
		       enum mux_setting setting)
{
	unsigned long shd_mux = pc302_read_from_register(SHD_GPIO_MUX_REG);

	if (MUX_ARM == setting)
		shd_mux |= (1 << arm_pin_nr);
	else
		shd_mux &= ~(1 << arm_pin_nr);

	pc302_write_to_register(SHD_GPIO_MUX_REG, shd_mux);
}

static int
pc3xx_get_shd_mux(struct muxed_pin *pin)
{
	unsigned long shd_mux = pc302_read_from_register(SHD_GPIO_MUX_REG);
	unsigned long syscfg = syscfg_read();

	if (pin->sd_pin == PC3X3_GPIO_PIN_SDGPIO_0) {
		if (syscfg & AXI2CFG_SYS_CONFIG_FREQ_SYNTH_MUX_MASK)
			return MUX_PERIPHERAL;
		else
			return MUX_SD;
	}

	return shd_mux & (1 << pin->arm_pin) ? MUX_ARM : MUX_SD;
}

/*
 * Shared ARM/SD gpio pins. These pins go to the arm_gpio[3:0] pads but can
 * be arm or sdgpio.
 */
static int
pc3xx_shd_mux(struct muxed_pin *pin,
	      enum mux_setting setting)
{
	if (MUX_PERIPHERAL == setting) {
		if (pin->sd_pin != PC3X3_GPIO_PIN_SDGPIO_0)
			return -EINVAL;

		syscfg_update(AXI2CFG_SYS_CONFIG_FREQ_SYNTH_MUX_MASK,
			      AXI2CFG_SYS_CONFIG_FREQ_SYNTH_MUX_MASK);
	} else if (PC3X3_GPIO_PIN_SDGPIO_0 == pin->sd_pin) {
		syscfg_update(AXI2CFG_SYS_CONFIG_FREQ_SYNTH_MUX_MASK, 0);
	}

	if (PC3X3_GPIO_PIN_SDGPIO_0 == pin->sd_pin && MUX_ARM == setting)
		return -EINVAL;

	pc3xx_shd_gpio_set_mux(pin->arm_pin, setting);

	return 0;
}

static struct muxed_pin armgpio_0_4[] = {
GPIO(arm_gpio0, PC3X3_GPIO_PIN_SDGPIO_16, PC3X3_GPIO_PIN_ARM_0, pc3xx_shd_mux,
     pc3xx_get_shd_mux),
GPIO(arm_gpio1, PC3X3_GPIO_PIN_SDGPIO_17, PC3X3_GPIO_PIN_ARM_1, pc3xx_shd_mux,
     pc3xx_get_shd_mux),
GPIO(arm_gpio2, PC3X3_GPIO_PIN_SDGPIO_18, PC3X3_GPIO_PIN_ARM_2, pc3xx_shd_mux,
     pc3xx_get_shd_mux),
GPIO(arm_gpio3, PC3X3_GPIO_PIN_SDGPIO_19, PC3X3_GPIO_PIN_ARM_3, pc3xx_shd_mux,
     pc3xx_get_shd_mux),
};

static struct pin_group armgpio_0_4_group = {
	.nr_pins    = ARRAY_SIZE(armgpio_0_4),
	.name	    = "arm_gpio[3:0]",
	.pins	    = armgpio_0_4,
};

static struct muxed_pin shd_gpio[] = {
GPIO(shd_gpio, PC3X3_GPIO_PIN_SDGPIO_8, PC3X3_GPIO_PIN_ARM_8, pc3xx_shd_mux,
     pc3xx_get_shd_mux),
};

static struct pin_group pc3x3_shd_group = {
	.nr_pins    = ARRAY_SIZE(shd_gpio),
	.name	    = "shd_gpio",
	.pins	    = shd_gpio,
};

/*
 * boot_mode[1:0] pads - the pins switch to gpio automatically after boot and
 * can be either arm or sdgpio.
 */
static struct muxed_pin boot_mode_0_1[] = {
GPIO(boot_mode0, PC3X3_GPIO_PIN_SDGPIO_9, PC3X3_GPIO_PIN_ARM_9,
     pc3xx_shd_mux, pc3xx_get_shd_mux),
GPIO(boot_mode1, PC3X3_GPIO_PIN_SDGPIO_10, PC3X3_GPIO_PIN_ARM_10,
     pc3xx_shd_mux, pc3xx_get_shd_mux),
};

static struct pin_group boot_mode_group = {
	.nr_pins    = ARRAY_SIZE(boot_mode_0_1),
	.name	    = "boot_mode[1:0]",
	.pins	    = boot_mode_0_1,
};

/*
 * sdram_speed_sel pad - automatically switches to gpio after boot and can be
 * arm or sdgpio.
 */
static struct muxed_pin sdram_speed_sel[] = {
GPIO(sdram_speed_sel, PC3X3_GPIO_PIN_SDGPIO_11, PC3X3_GPIO_PIN_ARM_11,
     pc3xx_shd_mux, pc3xx_get_shd_mux),
};

static struct pin_group sdram_speed_sel_group = {
	.nr_pins    = ARRAY_SIZE(sdram_speed_sel),
	.name	    = "sdram_speed_sel",
	.pins	    = sdram_speed_sel,
};

/*
 * mii_rev_en pad - automatically switches to gpio after boot and can be
 * arm or sdgpio.
 */
static struct muxed_pin mii_rev_en[] = {
GPIO(mii_rev_en, PC3X3_GPIO_PIN_SDGPIO_12, PC3X3_GPIO_PIN_ARM_12,
     pc3xx_shd_mux, pc3xx_get_shd_mux),
};

static struct pin_group mii_rev_en_group = {
	.nr_pins    = ARRAY_SIZE(mii_rev_en),
	.name	    = "mii_rev_en",
	.pins	    = mii_rev_en,
};

/*
 * mii_rmii_en pad - automatically switches to gpio after boot and can be
 * arm or sdgpio.
 */
static struct muxed_pin mii_rmii_en[] = {
GPIO(mii_rmii_en, PC3X3_GPIO_PIN_SDGPIO_13, PC3X3_GPIO_PIN_ARM_13,
     pc3xx_shd_mux, pc3xx_get_shd_mux),
};

static struct pin_group mii_rmii_en_group = {
	.nr_pins    = ARRAY_SIZE(mii_rmii_en),
	.name	    = "mii_rmii_en",
	.pins	    = mii_rmii_en,
};

/*
 * mii_speed_sel pad - automatically switches to gpio after boot and can be
 * arm or sdgpio.
 */
static struct muxed_pin mii_speed_sel[] = {
GPIO(mii_speed_sel, PC3X3_GPIO_PIN_SDGPIO_14, PC3X3_GPIO_PIN_ARM_14,
     pc3xx_shd_mux, pc3xx_get_shd_mux),
};

static struct pin_group mii_speed_sel_group = {
	.nr_pins    = ARRAY_SIZE(mii_speed_sel),
	.name	    = "mii_speed_sel",
	.pins	    = mii_speed_sel,
};

static int
pc3x3_shd_ebi_get_mux(struct muxed_pin *pin)
{
	int ebi_pin, err = 0, can_be_sd = 1;
	unsigned long ebi_mux;

	ebi_mux = pc302_read_from_register(USE_EBI_GPIO_REG);

	/*
	 * Find out what EBI pin our GPIO maps to.
	 */
	if (pin->arm_pin >= PC3X3_GPIO_PIN_ARM_4 &&
	    pin->arm_pin < PC3X3_GPIO_PIN_ARM_8) {
		ebi_pin = 22 + (pin->arm_pin - PC3X3_GPIO_PIN_ARM_4);
	} else if (pin->arm_pin >= PC3X3_GPIO_PIN_ARM_20 &&
		   pin->arm_pin < PC3X3_GPIO_PIN_ARM_24) {
		ebi_pin = 18 + (pin->arm_pin - PC3X3_GPIO_PIN_ARM_20);
	} else {
		/* These aren't shared gpio pins. */
		ebi_pin = 14 + (pin->arm_pin - PC3X3_GPIO_PIN_ARM_14);
		can_be_sd = 0;
	}

	if (!(ebi_mux & (1 << (ebi_pin - 14)))) {
		err = MUX_PERIPHERAL;
		goto out;
	}

	if (can_be_sd)
		err = pc3xx_get_shd_mux(pin);
	else
		err = MUX_ARM;

out:
	return err;
}

static int
pc3x3_shd_ebi_set_mux(struct muxed_pin *pin,
		      enum mux_setting setting)
{
	int ebi_pin, err = 0, can_be_sd = 1;
	unsigned long ebi_mux;

	ebi_mux = pc302_read_from_register(USE_EBI_GPIO_REG);

	/*
	 * Find out what EBI pin our GPIO maps to.
	 */
	if (pin->arm_pin >= PC3X3_GPIO_PIN_ARM_4 &&
	    pin->arm_pin < PC3X3_GPIO_PIN_ARM_8) {
		ebi_pin = 22 + (pin->arm_pin - PC3X3_GPIO_PIN_ARM_4);
	} else if (pin->arm_pin >= PC3X3_GPIO_PIN_ARM_20 &&
		   pin->arm_pin < PC3X3_GPIO_PIN_ARM_24) {
		ebi_pin = 18 + (pin->arm_pin - PC3X3_GPIO_PIN_ARM_20);
	} else {
		/* These aren't shared gpio pins. */
		ebi_pin = 14 + (pin->arm_pin - PC3X3_GPIO_PIN_ARM_14);
		can_be_sd = 0;
	}

	if (MUX_SD == setting && !can_be_sd) {
		err = -EINVAL;
		goto out;
	}

	/* Use the pin as EBI. */
	if (MUX_PERIPHERAL == setting)
		ebi_mux &= ~(1 << (ebi_pin - 14));
	else
		ebi_mux |= (1 << (ebi_pin - 14));

	pc302_write_to_register(USE_EBI_GPIO_REG, ebi_mux);

	/*
	 * Make sure that the configuration is valid (the GPIO isn't going to
	 * the PAI).
	 */
	if (pc302_read_from_register(USE_EBI_GPIO_REG) != ebi_mux) {
		err = -EBUSY;
		goto out;
	}

	/*
	 * If we can be SD or ARM and we want to be gpio, pick the correct
	 * one now.
	 */
	if (MUX_PERIPHERAL != setting && can_be_sd)
		pc3xx_shd_gpio_set_mux(pin->arm_pin, setting);

out:
	return err;
}

/*
 * ebi_addr[25:18] pads - these pads can be either the EBI or arm gpio or
 * sdgpio. Note: the gpio pins can also be routed to the pai_tx/rx_data pads.
 *
 * A pad may only be used for GPIO if the corresponding GPIO pin is not
 * already routed to the pai tx/rx data pad. The same applies in the reverse
 * direction. Hardware interlocks exist to prevent this from happening.
 */
static struct muxed_pin ebi_addr_18_25[] = {
PIN(ebi_addr18, PC3X3_GPIO_PIN_SDGPIO_4, PC3X3_GPIO_PIN_ARM_20,
    pc3x3_shd_ebi_set_mux, pc3x3_shd_ebi_get_mux),
PIN(ebi_addr19, PC3X3_GPIO_PIN_SDGPIO_5, PC3X3_GPIO_PIN_ARM_21,
    pc3x3_shd_ebi_set_mux, pc3x3_shd_ebi_get_mux),
PIN(ebi_addr20, PC3X3_GPIO_PIN_SDGPIO_6, PC3X3_GPIO_PIN_ARM_22,
    pc3x3_shd_ebi_set_mux, pc3x3_shd_ebi_get_mux),
PIN(ebi_addr21, PC3X3_GPIO_PIN_SDGPIO_7, PC3X3_GPIO_PIN_ARM_23,
    pc3x3_shd_ebi_set_mux, pc3x3_shd_ebi_get_mux),
PIN(ebi_addr22, PC3X3_GPIO_PIN_SDGPIO_20, PC3X3_GPIO_PIN_ARM_4,
    pc3x3_shd_ebi_set_mux, pc3x3_shd_ebi_get_mux),
PIN(ebi_addr23, PC3X3_GPIO_PIN_SDGPIO_21, PC3X3_GPIO_PIN_ARM_5,
    pc3x3_shd_ebi_set_mux, pc3x3_shd_ebi_get_mux),
PIN(ebi_addr24, PC3X3_GPIO_PIN_SDGPIO_22, PC3X3_GPIO_PIN_ARM_6,
    pc3x3_shd_ebi_set_mux, pc3x3_shd_ebi_get_mux),
PIN(ebi_addr25, PC3X3_GPIO_PIN_SDGPIO_23, PC3X3_GPIO_PIN_ARM_7,
    pc3x3_shd_ebi_set_mux, pc3x3_shd_ebi_get_mux),
};

static struct pin_group ebi_addr_18_25_group = {
	.nr_pins    = ARRAY_SIZE(ebi_addr_18_25),
	.name	    = "ebi_addr[25:18]",
	.pins	    = ebi_addr_18_25,
};

static int
pc3x3_shd_pai_get_mux(struct muxed_pin *pin)
{
	int bit, err = 0, can_be_sd = 1;
	unsigned long pai_mux;

	pai_mux = pc302_read_from_register(USE_PAI_GPIO_REG);

	/*
	 * Find out what pai pin our GPIO maps to.
	 */
	if (pin->arm_pin >= PC3X3_GPIO_PIN_ARM_4 &&
	    pin->arm_pin < PC3X3_GPIO_PIN_ARM_8) {
		/* pai_tx_data[3:0] */
		bit = pin->arm_pin - PC3X3_GPIO_PIN_ARM_4;
	} else if (pin->arm_pin >= PC3X3_GPIO_PIN_ARM_20 &&
		   pin->arm_pin < PC3X3_GPIO_PIN_ARM_24) {
		/* pai_rx_data[3:0] */
		bit = 8 + (pin->arm_pin - PC3X3_GPIO_PIN_ARM_20);
	} else if (pin->arm_pin >= PC3X3_GPIO_PIN_ARM_24 &&
		   pin->arm_pin < PC3X3_GPIO_PIN_ARM_28) {
		/* pai_tx_data[7:4] */
		bit = 4 + (pin->arm_pin - PC3X3_GPIO_PIN_ARM_24);
		can_be_sd = 0;
	} else {
		/* pai_rx_data[7:4] */
		bit = 12 + (pin->arm_pin - PC3X3_GPIO_PIN_ARM_28);
		can_be_sd = 0;
	}

	if (!(pai_mux & (1 << bit))) {
		err = MUX_PERIPHERAL;
		goto out;
	}

	if (can_be_sd)
		err = pc3xx_get_shd_mux(pin);
	else
		err = MUX_ARM;

out:
	return err;
}

static int
pc3x3_shd_pai_set_mux(struct muxed_pin *pin,
		      enum mux_setting setting)
{
	int bit, err = 0, can_be_sd = 1;
	unsigned long pai_mux;

	pai_mux = pc302_read_from_register(USE_PAI_GPIO_REG);

	/*
	 * Find out what pai pin our GPIO maps to.
	 */
	if (pin->arm_pin >= PC3X3_GPIO_PIN_ARM_4 &&
	    pin->arm_pin < PC3X3_GPIO_PIN_ARM_8) {
		/* pai_tx_data[3:0] */
		bit = pin->arm_pin - PC3X3_GPIO_PIN_ARM_4;
	} else if (pin->arm_pin >= PC3X3_GPIO_PIN_ARM_20 &&
		   pin->arm_pin < PC3X3_GPIO_PIN_ARM_24) {
		/* pai_rx_data[3:0] */
		bit = 8 + (pin->arm_pin - PC3X3_GPIO_PIN_ARM_20);
	} else if (pin->arm_pin >= PC3X3_GPIO_PIN_ARM_24 &&
		   pin->arm_pin < PC3X3_GPIO_PIN_ARM_28) {
		/* pai_tx_data[7:4] */
		bit = 4 + (pin->arm_pin - PC3X3_GPIO_PIN_ARM_24);
		can_be_sd = 0;
	} else {
		/* pai_rx_data[7:4] */
		bit = 12 + (pin->arm_pin - PC3X3_GPIO_PIN_ARM_28);
		can_be_sd = 0;
	}

	if (MUX_SD == setting && !can_be_sd) {
		err = -EINVAL;
		goto out;
	}

	/* Use the pin as pai. */
	if (MUX_PERIPHERAL == setting)
		pai_mux &= ~(1 << bit);
	else
		pai_mux |= (1 << bit);

	pc302_write_to_register(USE_PAI_GPIO_REG, pai_mux);

	/*
	 * Make sure that the configuration is valid (the GPIO isn't going to
	 * the EBI).
	 */
	if (pc302_read_from_register(USE_PAI_GPIO_REG) != pai_mux) {
		err = -EBUSY;
		goto out;
	}

	/*
	 * If we can be SD or ARM and we want to be gpio, pick the correct
	 * one now.
	 */
	if (MUX_PERIPHERAL != setting && can_be_sd)
		pc3xx_shd_gpio_set_mux(pin->arm_pin, setting);

out:
	return err;
}

/*
 * pai_rx_data[3:0] pads - these pads can be either the pai_rx_data or arm
 * gpio or sdgpio. Note: the gpio pins can also be routed to the
 * ebi_addr pads.
 *
 * A pad may only be used for GPIO if the corresponding GPIO pin is not
 * already routed to the ebi address pad. The same applies in the reverse
 * direction. Hardware interlocks exist to prevent this from happening.
 */
static struct muxed_pin pai_rx_data_0_3[] = {
PIN(pai_rx_data0, PC3X3_GPIO_PIN_SDGPIO_4, PC3X3_GPIO_PIN_ARM_20,
    pc3x3_shd_pai_set_mux, pc3x3_shd_pai_get_mux),
PIN(pai_rx_data1, PC3X3_GPIO_PIN_SDGPIO_5, PC3X3_GPIO_PIN_ARM_21,
    pc3x3_shd_pai_set_mux, pc3x3_shd_pai_get_mux),
PIN(pai_rx_data2, PC3X3_GPIO_PIN_SDGPIO_6, PC3X3_GPIO_PIN_ARM_22,
    pc3x3_shd_pai_set_mux, pc3x3_shd_pai_get_mux),
PIN(pai_rx_data3, PC3X3_GPIO_PIN_SDGPIO_7, PC3X3_GPIO_PIN_ARM_23,
    pc3x3_shd_pai_set_mux, pc3x3_shd_pai_get_mux),
};

static struct pin_group pai_rx_data_0_3_group = {
	.nr_pins    = ARRAY_SIZE(pai_rx_data_0_3),
	.name	    = "pai_rx_data[3:0]",
	.pins	    = pai_rx_data_0_3,
};

/*
 * pai_tx_data[3:0] pads - these pads can be either the pai_tx_data or arm
 * gpio or sdgpio. Note: the gpio pins can also be routed to the
 * ebi_addr pads.
 *
 * A pad may only be used for GPIO if the corresponding GPIO pin is not
 * already routed to the ebi address pad. The same applies in the reverse
 * direction. Hardware interlocks exist to prevent this from happening.
 */
static struct muxed_pin pai_tx_data_0_3[] = {
PIN(pai_tx_data0, PC3X3_GPIO_PIN_SDGPIO_20, PC3X3_GPIO_PIN_ARM_4,
    pc3x3_shd_pai_set_mux, pc3x3_shd_pai_get_mux),
PIN(pai_tx_data1, PC3X3_GPIO_PIN_SDGPIO_21, PC3X3_GPIO_PIN_ARM_5,
    pc3x3_shd_pai_set_mux, pc3x3_shd_pai_get_mux),
PIN(pai_tx_data2, PC3X3_GPIO_PIN_SDGPIO_22, PC3X3_GPIO_PIN_ARM_6,
    pc3x3_shd_pai_set_mux, pc3x3_shd_pai_get_mux),
PIN(pai_tx_data3, PC3X3_GPIO_PIN_SDGPIO_23, PC3X3_GPIO_PIN_ARM_7,
    pc3x3_shd_pai_set_mux, pc3x3_shd_pai_get_mux),
};

static struct pin_group pai_tx_data_0_3_group = {
	.nr_pins    = ARRAY_SIZE(pai_tx_data_0_3),
	.name	    = "pai_tx_data[3:0]",
	.pins	    = pai_tx_data_0_3,
};

/*
 * pai_tx_data[7:4] pads - these pads can either be pai_tx_data or arm gpio.
 */
static struct muxed_pin pai_tx_data_4_7[] = {
PIN(pai_tx_data4, -1, PC3X3_GPIO_PIN_ARM_24, pc3x3_shd_pai_set_mux,
    pc3x3_shd_pai_get_mux),
PIN(pai_tx_data5, -1, PC3X3_GPIO_PIN_ARM_25, pc3x3_shd_pai_set_mux,
    pc3x3_shd_pai_get_mux),
PIN(pai_tx_data6, -1, PC3X3_GPIO_PIN_ARM_26, pc3x3_shd_pai_set_mux,
    pc3x3_shd_pai_get_mux),
PIN(pai_tx_data7, -1, PC3X3_GPIO_PIN_ARM_27, pc3x3_shd_pai_set_mux,
    pc3x3_shd_pai_get_mux),
};

static struct pin_group pai_tx_data_4_7_group = {
	.nr_pins    = ARRAY_SIZE(pai_tx_data_4_7),
	.name	    = "pai_tx_data[7:4]",
	.pins	    = pai_tx_data_4_7,
};

/*
 * pai_rx_data[7:4] pads - these pads can either be pai_rx_data or arm gpio.
 */
static struct muxed_pin pai_rx_data_4_7[] = {
PIN(pai_rx_data4, -1, PC3X3_GPIO_PIN_ARM_28, pc3x3_shd_pai_set_mux,
    pc3x3_shd_pai_get_mux),
PIN(pai_rx_data5, -1, PC3X3_GPIO_PIN_ARM_29, pc3x3_shd_pai_set_mux,
    pc3x3_shd_pai_get_mux),
PIN(pai_rx_data6, -1, PC3X3_GPIO_PIN_ARM_30, pc3x3_shd_pai_set_mux,
    pc3x3_shd_pai_get_mux),
PIN(pai_rx_data7, -1, PC3X3_GPIO_PIN_ARM_31, pc3x3_shd_pai_set_mux,
    pc3x3_shd_pai_get_mux),
};

static struct pin_group pai_rx_data_4_7_group = {
	.nr_pins    = ARRAY_SIZE(pai_rx_data_4_7),
	.name	    = "pai_rx_data[7:4]",
	.pins	    = pai_rx_data_4_7,
};

/*
 * ebi_addr[17:14] pads - these pads can either be ebi_addr or arm gpio.
 */
static struct muxed_pin ebi_addr_14_17[] = {
PIN(ebi_addr14, -1, PC3X3_GPIO_PIN_ARM_32, pc3x3_shd_ebi_set_mux,
    pc3x3_shd_ebi_get_mux),
PIN(ebi_addr15, -1, PC3X3_GPIO_PIN_ARM_33, pc3x3_shd_ebi_set_mux,
    pc3x3_shd_ebi_get_mux),
PIN(ebi_addr16, -1, PC3X3_GPIO_PIN_ARM_34, pc3x3_shd_ebi_set_mux,
    pc3x3_shd_ebi_get_mux),
PIN(ebi_addr17, -1, PC3X3_GPIO_PIN_ARM_35, pc3x3_shd_ebi_set_mux,
    pc3x3_shd_ebi_get_mux),
};

static struct pin_group ebi_addr_14_17_group = {
	.nr_pins    = ARRAY_SIZE(ebi_addr_14_17),
	.name	    = "ebi_addr[17:14]",
	.pins	    = ebi_addr_14_17,
};

static int
decode_get_mux(struct muxed_pin *pin)
{
	unsigned bit = 1 << (pin->arm_pin - PC3X3_GPIO_PIN_ARM_36);
	unsigned long use_decode_gpio =
            pc302_read_from_register(USE_DECODE_GPIO_REG);

	return use_decode_gpio & (1 << bit) ? MUX_ARM : MUX_PERIPHERAL;
}

static int
decode_set_mux(struct muxed_pin *pin,
	       enum mux_setting setting)
{
	unsigned long use_decode_gpio;
	unsigned bit = pin->arm_pin == PC3X3_GPIO_PIN_ARM_36 ? 0 : 1;

	if (MUX_SD == setting)
		return -EINVAL;

	use_decode_gpio = pc302_read_from_register(USE_DECODE_GPIO_REG);
	if (MUX_ARM == setting)
		use_decode_gpio |= (1 << bit);
	else
		use_decode_gpio &= ~(1 << bit);
	pc302_write_to_register(USE_DECODE_GPIO_REG, use_decode_gpio);

	return 0;
}

/*
 * decode[3:0] pads - these pads can either be decode pins or arm gpio.
 */
static struct muxed_pin decode_0_3[] = {
PIN(decode0, -1, PC3X3_GPIO_PIN_ARM_36, decode_set_mux, decode_get_mux),
PIN(decode1, -1, PC3X3_GPIO_PIN_ARM_37, decode_set_mux, decode_get_mux),
PIN(decode2, -1, PC3X3_GPIO_PIN_ARM_38, decode_set_mux, decode_get_mux),
PIN(decode3, -1, PC3X3_GPIO_PIN_ARM_39, decode_set_mux, decode_get_mux),
};

static struct pin_group decode_0_3_group = {
	.nr_pins    = ARRAY_SIZE(decode_0_3),
	.name	    = "decode[3:0]",
	.pins	    = decode_0_3,
};

static int
ssi_set_mux(struct muxed_pin *pin,
	    enum mux_setting setting);

static int
ssi_get_mux(struct muxed_pin *pin);

/*
 * ssi pads - these pads can either be ssi block pins or arm gpio.
 */
static struct muxed_pin ssi[] = {
PIN(ssi_clk, -1, PC3X3_GPIO_PIN_ARM_40, ssi_set_mux, ssi_get_mux),
PIN(ssi_data_in, -1, PC3X3_GPIO_PIN_ARM_41, ssi_set_mux, ssi_get_mux),
PIN(ssi_data_out, -1, PC3X3_GPIO_PIN_ARM_42, ssi_set_mux, ssi_get_mux),
};

static int
ssi_get_mux(struct muxed_pin *pin)
{
	unsigned long use_misc_int_gpio =
            pc302_read_from_register(USE_MISC_INT_GPIO_REG);

	return use_misc_int_gpio & (1 << 0) ? MUX_ARM : MUX_PERIPHERAL;
}

static int
ssi_set_mux(struct muxed_pin *pin,
	    enum mux_setting setting)
{
	unsigned long use_misc_int_gpio;

	if (MUX_SD == setting)
		return -EINVAL;

	use_misc_int_gpio = pc302_read_from_register(USE_MISC_INT_GPIO_REG);
	if (MUX_PERIPHERAL == setting)
		use_misc_int_gpio &= ~(1 << 0);
	else
		use_misc_int_gpio |= (1 << 0);
	pc302_write_to_register(USE_MISC_INT_GPIO_REG, use_misc_int_gpio);

	return 0;
}

static struct pin_group ssi_group = {
	.nr_pins    = ARRAY_SIZE(ssi),
	.name	    = "ssi",
	.pins	    = ssi,
};

static int
mii_get_mux(struct muxed_pin *pin)
{
	unsigned long syscfg = syscfg_read();

	return syscfg & (1 << 13) ? MUX_ARM : MUX_PERIPHERAL;
}

static int
mii_set_mux(struct muxed_pin *pin,
	    enum mux_setting setting)
{
	/*
	 * These are automatically configured by hardware if we are in
	 * reduced MII mode.
	 */
	return -EOPNOTSUPP;
}

/*
 * mii pads - these pads can either be mii pins or arm gpio.
 */
static struct muxed_pin mii[] = {
PIN(mii_tx_data2, -1, PC3X3_GPIO_PIN_ARM_43, mii_set_mux, mii_get_mux),
PIN(mii_tx_data3, -1, PC3X3_GPIO_PIN_ARM_44, mii_set_mux, mii_get_mux),
PIN(mii_rx_data2, -1, PC3X3_GPIO_PIN_ARM_45, mii_set_mux, mii_get_mux),
PIN(mii_rx_data3, -1, PC3X3_GPIO_PIN_ARM_46, mii_set_mux, mii_get_mux),
PIN(mii_col, -1, PC3X3_GPIO_PIN_ARM_47, mii_set_mux, mii_get_mux),
PIN(mii_crs, -1, PC3X3_GPIO_PIN_ARM_48, mii_set_mux, mii_get_mux),
PIN(mii_tx_clk, -1, PC3X3_GPIO_PIN_ARM_49, mii_set_mux, mii_get_mux),
};

static struct pin_group mii_group = {
	.nr_pins    = ARRAY_SIZE(mii),
	.name	    = "mii",
	.pins	    = mii,
};

static int
max_set_mux(struct muxed_pin *pin,
	    enum mux_setting setting);

static int
max_get_mux(struct muxed_pin *pin);

/*
 * maxim pads - these pads can either be maxim pins or arm gpio.
 */
static struct muxed_pin max[] = {
PIN(max_tx_ctrl, -1, PC3X3_GPIO_PIN_ARM_50, max_set_mux, max_get_mux),
PIN(max_ref_clk, -1, PC3X3_GPIO_PIN_ARM_51, max_set_mux, max_get_mux),
PIN(max_trig_clk, -1, PC3X3_GPIO_PIN_ARM_52, max_set_mux, max_get_mux),
};

static int
max_get_mux(struct muxed_pin *pin)
{
	unsigned long use_misc_int_gpio =
            pc302_read_from_register(USE_MISC_INT_GPIO_REG);

	return use_misc_int_gpio & (1 << 1) ? MUX_ARM : MUX_PERIPHERAL;
}

static int
max_set_mux(struct muxed_pin *pin,
	    enum mux_setting setting)
{
	unsigned long use_misc_int_gpio;

	if (MUX_SD == setting)
		return -EINVAL;

	use_misc_int_gpio = pc302_read_from_register(USE_MISC_INT_GPIO_REG);
	if (MUX_PERIPHERAL == setting)
		use_misc_int_gpio &= ~(1 << 1);
	else
		use_misc_int_gpio |= (1 << 1);
	pc302_write_to_register(USE_MISC_INT_GPIO_REG, use_misc_int_gpio);

	return 0;
}


static struct pin_group max_group = {
	.nr_pins    = ARRAY_SIZE(max),
	.name	    = "mii",
	.pins	    = max,
};

static int
ebi_clk_get_mux(struct muxed_pin *pin)
{
	unsigned long ebi_mux = pc302_read_from_register(USE_EBI_GPIO_REG);

	return ebi_mux & (1 << 13) ? MUX_ARM : MUX_PERIPHERAL;
}

static int
ebi_clk_set_mux(struct muxed_pin *pin,
	        enum mux_setting setting)
{
	unsigned long ebi_mux;

	if (MUX_SD == setting)
		return -EINVAL;

	ebi_mux = pc302_read_from_register(USE_EBI_GPIO_REG);
	if (MUX_PERIPHERAL == setting)
		ebi_mux &= ~(1 << 13);
	else
		ebi_mux |= (1 << 13);
	pc302_write_to_register(USE_EBI_GPIO_REG, ebi_mux);

	return 0;
}

/*
 * ebi clock pads - this pad can either be the ebi clock or an arm gpio.
 */
static struct muxed_pin ebi_clk[] = {
PIN(ebi_clk, -1, PC3X3_GPIO_PIN_ARM_53, ebi_clk_set_mux, ebi_clk_get_mux),
};

static struct pin_group ebi_clk_group = {
	.nr_pins    = ARRAY_SIZE(ebi_clk),
	.name	    = "ebi_clk",
	.pins	    = ebi_clk,
};

static struct muxed_pin pc3x3_fracn_pins[] = {
PIN(sdgpio0, PC3X3_GPIO_PIN_SDGPIO_0, -1, pc3xx_shd_mux, pc3xx_get_shd_mux),
};

static struct pin_group pc3x3_fracn_group = {
	.nr_pins    = ARRAY_SIZE(pc3x3_fracn_pins),
	.name	    = "fracn/sdgpio0",
	.pins	    = pc3x3_fracn_pins,
};

static struct pin_group *pc3x3_groups[] = {
	&armgpio_0_4_group,
	&pc3x3_shd_group,
	&boot_mode_group,
	&sdram_speed_sel_group,
	&mii_rev_en_group,
	&mii_rmii_en_group,
	&mii_speed_sel_group,
	&ebi_addr_18_25_group,
	&pai_tx_data_0_3_group,
	&pai_rx_data_0_3_group,
	&pai_tx_data_4_7_group,
	&pai_rx_data_4_7_group,
	&ebi_addr_14_17_group,
	&decode_0_3_group,
	&ssi_group,
	&mii_group,
	&max_group,
	&ebi_clk_group,
	&pc3x3_fracn_group,
};
#endif /* CONFIG_PICOCHIP_PC3X3 */

/* Public API -------------------------------------------------------------- */
/*****************************************************************************
 * Generic level pin multiplexing.
 ****************************************************************************/

static struct {
	unsigned	    num_groups;
	struct pin_group    **groups;
} all_groups;

int
pc3xx_pin_set_mux(int pin_nr,
		  enum mux_setting setting)
{
	unsigned i, j;
	int ret = 0;

	/*
	 * Don't let users try and trick us - they can't change the hardware
	 * that much!
	 */
	if (MUX_UNMUXED == setting)
		return -EINVAL;

	for (i = 0; i < all_groups.num_groups; ++i) {
		struct pin_group *group = all_groups.groups[i];
		for (j = 0; j < group->nr_pins; ++j) {
			struct muxed_pin *pin = &group->pins[j];
			/*
			 * Dedicated GPIO pins aren't shared with a
			 * peripheral. This is illegal!
			 */
			if (pin->is_dedicated_gpio &&
			    MUX_PERIPHERAL == setting)
				return -EINVAL;
			if (pin_nr == pin->arm_pin ||
			    pin_nr == pin->sd_pin) {
				ret = pin->set_mux(pin, setting);
				if (!ret)
					goto out;
				/*
				 * If we failed to set the muxing of this pin,
				 * carry on looping as we have some
				 * many-to-many pins so we might pick it up
				 * again on another output.
				 */
			}
		}
	}

	/*
	 * If we don't have a multiplexed pin entry for the requested pin then
	 * we assume that the pin isn't multiplexed so we don't need to do
	 * anything.
	 */
out:
	return ret;
}

int
pc3xx_group_set_mux(const char *group_name,
		    enum mux_setting setting)
{
	unsigned i, j;
	int err = -ENXIO;

	/*
	 * Don't let users try and trick us - they can't change the hardware
	 * that much!
	 */
	if (MUX_UNMUXED == setting)
		return -EINVAL;

	for (i = 0; i < all_groups.num_groups; ++i) {
		struct pin_group *group = all_groups.groups[i];
		if (strcmp(group->name, group_name))
			continue;

		for (j = 0; j < group->nr_pins; ++j) {
			struct muxed_pin *pin = &group->pins[j];
			/*
			 * Dedicated GPIO pins aren't shared with a
			 * peripheral. This is illegal!
			 */
			if (pin->is_dedicated_gpio &&
			    MUX_PERIPHERAL == setting)
				return -EINVAL;
			err = pin->set_mux(pin, setting);
			if (err)
				goto out;
		}

		break;
	}

out:
	return err;
}

int
pc3xx_get_pin_mux(int pin_nr)
{
	unsigned i, j;
	int ret = 0;

	for (i = 0; i < all_groups.num_groups; ++i) {
		struct pin_group *group = all_groups.groups[i];
		for (j = 0; j < group->nr_pins; ++j) {
			struct muxed_pin *pin = &group->pins[j];
			if (pin_nr == pin->arm_pin ||
			    pin_nr == pin->sd_pin) {
				int tmp = pin->get_mux(pin);
				if (tmp < 0)
					return tmp;
				ret |= tmp;
			}
		}
	}

	/*
	 * If we don't have a multiplexed pin entry for the requested pin then
	 * we assume that the pin isn't multiplexed.
	 */
	return ret ? ret : MUX_UNMUXED;
}

void
pc3xx_muxing_init(void)
{
	if (!is_pc3x3()) {
#ifdef CONFIG_PICOCHIP_PC3X2
		all_groups.num_groups = ARRAY_SIZE(pc3x2_groups);
		all_groups.groups = pc3x2_groups;
#endif /* CONFIG_PICOCHIP_PC3X2 */
	} else {
#ifdef CONFIG_PICOCHIP_PC3X3
		all_groups.num_groups = ARRAY_SIZE(pc3x3_groups);
		all_groups.groups = pc3x3_groups;
#endif /* CONFIG_PICOCHIP_PC3X3 */
	}
}
