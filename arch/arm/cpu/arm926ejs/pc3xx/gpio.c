/*****************************************************************************
 * $picoChipHeaderSubst$
 *****************************************************************************/

/*
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
#include <asm/arch/utilities.h>
#include <asm/arch/gpio.h>
#include <asm/arch/mux.h>
#include <asm/arch/pc3xxgpio.h>

/* Macros ------------------------------------------------------------------ */
#define __iomem
#define _ioa (void __iomem *)
#define readl(addr) (pc302_read_from_register((unsigned int)addr))
#define writel(val, addr) (pc302_write_to_register((unsigned int)addr, val))

/* The number of gpio pins that this drover can handle */
#define ARCH_NR_GPIOS 128

#define BITS_PER_BYTE 8
#define BITS_TO_LONGS(nr) \
        DIV_ROUND_UP(nr, BITS_PER_BYTE * sizeof(long))

#define DECLARE_BITMAP(name,bits) \
        unsigned long name[BITS_TO_LONGS(bits)]

/* Constants --------------------------------------------------------------- */
/**
 * struct gpio_chip - abstract a GPIO controller
 * @label: for diagnostics
 * @request: optional hook for chip-specific activation, such as
 *	enabling module power and clock; may sleep
 * @free: optional hook for chip-specific deactivation, such as
 *	disabling module power and clock; may sleep
 * @direction_input: configures signal "offset" as input, or returns error
 * @get: returns value for signal "offset"; for output signals this
 *	returns either the value actually sensed, or zero
 * @direction_output: configures signal "offset" as output, or returns error
 * @set: assigns output value for signal "offset"
 * @base: identifies the first GPIO number handled by this chip; or, if
 *	negative during registration, requests dynamic ID allocation.
 * @ngpio: the number of GPIOs handled by this controller; the last GPIO
 *	handled is (base + ngpio - 1).
 * @names: if set, must be an array of strings to use as alternative
 *      names for the GPIOs in this chip. Any entry in the array
 *      may be NULL if there is no alias for the GPIO, however the
 *      array must be @ngpio entries long.
 *
 * A gpio_chip can help platforms abstract various sources of GPIOs so
 * they can all be accessed through a common programing interface.
 * Example sources would be SOC controllers, FPGAs, multifunction
 * chips, dedicated GPIO expanders, and so on.
 *
 * Each chip controls a number of signals, identified in method calls
 * by "offset" values in the range 0..(@ngpio - 1).  When those signals
 * are referenced through calls like gpio_get_value(gpio), the offset
 * is calculated by subtracting @base from the gpio number.
 */
struct gpio_chip {
	const char		*label;

	int			(*request)(struct gpio_chip *chip,
						unsigned offset);
	void			(*free)(struct gpio_chip *chip,
						unsigned offset);

	int			(*direction_input)(struct gpio_chip *chip,
						unsigned offset);
	int			(*get)(struct gpio_chip *chip,
						unsigned offset);
	int			(*direction_output)(struct gpio_chip *chip,
						unsigned offset, int value);
	void			(*set)(struct gpio_chip *chip,
						unsigned offset, int value);

	int			base;
	u16			ngpio;
	char			**names;
};

DECLARE_BITMAP(pin_status, ARCH_NR_GPIOS);

struct gpio_desc {
	struct gpio_chip	*chip;
};

static struct gpio_desc gpio_desc[ARCH_NR_GPIOS];

/* Functions --------------------------------------------------------------- */
static int
armgpio_request(struct gpio_chip *chip,
		unsigned offset)
{
	enum mux_setting mux;

	if (test_and_set_bit(offset + chip->base, pin_status))
		return -EBUSY;

	/* Check the pin has been correctly multiplexed. */
	mux = pc3xx_get_pin_mux(offset + chip->base);
	if (!(mux & (MUX_ARM | MUX_UNMUXED))) {
		/* The pin has an inconsistent mux setting. */
		printf("attempt to request armgpio%u which is not correctly multiplexed\n",
			   chip->base + offset);
		test_and_clear_bit(offset + chip->base, pin_status);
		return -EIO;
	}

	return 0;
}

static void
armgpio_free(struct gpio_chip *chip,
	     unsigned offset)
{
	test_and_clear_bit(offset + chip->base, pin_status);
}

#define GPIO_SW_PORT_A_EXT_REG_OFFSET	GPIO_EXT_PORT_A_REG_OFFSET
#define GPIO_SW_PORT_B_EXT_REG_OFFSET	GPIO_EXT_PORT_B_REG_OFFSET
#define GPIO_SW_PORT_D_EXT_REG_OFFSET	GPIO_EXT_PORT_D_REG_OFFSET

static inline int
armgpio_block_nr(unsigned gpio_nr)
{
	if (!is_pc3x3()) {
		/*
		 * PC3X2 has GPIOs numbered in non contiguous blocks.
		 */
		if (gpio_nr < PC302_GPIO_PIN_ARM_8)
			return gpio_nr - PC302_GPIO_PIN_ARM_0;
		else
			return gpio_nr - PC302_GPIO_PIN_ARM_8 + 8;
	} else {
		/*
		 * For PC3X3, ARM GPIO's are numbered from 0->MAX.
		 */
		return gpio_nr;
	}

	return -ENXIO;
}

#define __ARMGPIO_REG(_gpio_base, _reg)					    \
	({								    \
		void __iomem *ret = NULL;				    \
		int __gpio_nr = armgpio_block_nr(_gpio_base);		    \
		if (__gpio_nr < 8)					    \
			ret = _ioa(PC302_GPIO_BASE +			    \
				GPIO_SW_PORT_A_##_reg##_REG_OFFSET);	    \
		else if (__gpio_nr < 24)				    \
			ret = _ioa(PC302_GPIO_BASE +			    \
				GPIO_SW_PORT_B_##_reg##_REG_OFFSET);	    \
		else							    \
			ret = _ioa(PC302_GPIO_BASE +			    \
				GPIO_SW_PORT_D_##_reg##_REG_OFFSET);	    \
		ret;							    \
	})

#define ARMGPIO_DR(_gpio_base)	    __ARMGPIO_REG(_gpio_base, DR)
#define ARMGPIO_DDR(_gpio_base)	    __ARMGPIO_REG(_gpio_base, DDR)
#define ARMGPIO_CTL(_gpio_base)	    __ARMGPIO_REG(_gpio_base, CTL)
#define ARMGPIO_EXT(_gpio_base)	    __ARMGPIO_REG(_gpio_base, EXT)

static inline unsigned
armgpio_offset(unsigned offset)
{
	if (is_pc3x3()) {
		/*
		 * The arm gpios in PC3x3 are controlled via three sets of
		 * registers. The register addressing is already taken care
		 * of by the __ARMGPIO_REG macro, this takes care of the bit
		 * offsets within each register.
		 */
		if (offset < 8) /* GPIO Port A*/
			return offset;
		else if (offset < 24) /* GPIO Port B */
			return offset - 8;
		else /* GPIO Port D */
			return offset - 24;
	}

	return offset;
}

static int
armgpio_direction_input(struct gpio_chip *chip,
			unsigned offset)
{
	void __iomem *ddr = ARMGPIO_DDR(chip->base + offset);
	void __iomem *cr = ARMGPIO_CTL(chip->base + offset);
	unsigned long val, bit_offset = armgpio_offset(offset);

	/* Mark the pin as an output. */
	val = readl(ddr);
	val &= ~(1 << bit_offset);
        writel(val, ddr);

	/* Set the pin as software controlled. */
	val = readl(cr);
	val &= ~(1 << bit_offset);
        writel(val, cr);

	return 0;
}

static void
armgpio_set(struct gpio_chip *chip,
	    unsigned offset,
	    int value);

static int
armgpio_direction_output(struct gpio_chip *chip,
			 unsigned offset,
			 int value)
{
	void __iomem *ddr = ARMGPIO_DDR(chip->base + offset);
	void __iomem *cr = ARMGPIO_CTL(chip->base + offset);
	unsigned long val, bit_offset = armgpio_offset(offset);

	/* Set the value first so we don't glitch. */
	armgpio_set(chip, offset, value);

	/* Mark the pin as an output. */
	val = readl(ddr);
	val |= (1 << bit_offset);
        writel(val, ddr);

	/* Set the pin as software controlled. */
	val = readl(cr);
	val &= ~(1 << bit_offset);
	writel(val, cr);

	return 0;
}

static int
armgpio_get(struct gpio_chip *chip,
	    unsigned offset)
{
	void __iomem *ext = ARMGPIO_EXT(chip->base + offset);
	unsigned long bit_offset = armgpio_offset(offset);

	return !!(readl(ext) & (1 << bit_offset));
}

static void
armgpio_set(struct gpio_chip *chip,
	    unsigned offset,
	    int value)
{
	void __iomem *dr = ARMGPIO_DR(chip->base + offset);
	unsigned long val, bit_offset = armgpio_offset(offset);

	val = readl(dr);
	val &= ~(1 << bit_offset);
	val |= (!!value << bit_offset);
	writel(val, dr);
}

/*
 * PC3X2 ARM GPIO chips. One chip per port.
 */
#ifdef CONFIG_PICOCHIP_PC3X2
char *pc3x2_armgpio_lo_pins[] = {
	"arm0",
	"arm1",
	"arm2",
	"arm3",
	"arm4",
	"arm5",
	"arm6",
	"arm7",
};

static struct gpio_chip pc3x2_arm_gpio_lo = {
	.label		    = "armgpio_lo",
	.request	    = armgpio_request,
	.free		    = armgpio_free,
	.direction_input    = armgpio_direction_input,
	.direction_output   = armgpio_direction_output,
	.get		    = armgpio_get,
	.set		    = armgpio_set,
	.base		    = PC302_GPIO_PIN_ARM_0,
	.ngpio		    = ARRAY_SIZE(pc3x2_armgpio_lo_pins),
	.names		    = pc3x2_armgpio_lo_pins,
};

char *pc3x2_armgpio_shared_pins[] = {
	"arm8",
	"arm9",
	"arm10",
	"arm11",
	"arm12",
	"arm13",
	"arm14",
	"arm15",
};

static struct gpio_chip pc3x2_arm_gpio_shared = {
	.label		    = "armgpio_shared",
	.request	    = armgpio_request,
	.free		    = armgpio_free,
	.direction_input    = armgpio_direction_input,
	.direction_output   = armgpio_direction_output,
	.get		    = armgpio_get,
	.set		    = armgpio_set,
	.ngpio		    = ARRAY_SIZE(pc3x2_armgpio_shared_pins),
	.base		    = PC302_GPIO_PIN_ARM_8,
	.names		    = pc3x2_armgpio_shared_pins,
};
#endif /* CONFIG_PICOCHIP_PC3X2 */

#ifdef CONFIG_PICOCHIP_PC3X3
/*
 * PC3X3 ARM GPIO chip.
 */
char *pc3x3_armgpio_pins[] = {
	"arm0",
	"arm1",
	"arm2",
	"arm3",
	"arm4",
	"arm5",
	"arm6",
	"arm7",
	"arm8",
	"arm9",
	"arm10",
	"arm11",
	"arm12",
	"arm13",
	"arm14",
	"arm15",
	"arm16",
	"arm17",
	"arm18",
	"arm19",
	"arm20",
	"arm21",
	"arm22",
	"arm23",
	"arm24",
	"arm25",
	"arm26",
	"arm27",
	"arm28",
	"arm29",
	"arm30",
	"arm31",
	"arm32",
	"arm33",
	"arm34",
	"arm35",
	"arm36",
	"arm37",
	"arm38",
	"arm39",
	"arm40",
	"arm41",
	"arm42",
	"arm43",
	"arm44",
	"arm45",
	"arm46",
	"arm47",
	"arm48",
	"arm49",
	"arm50",
	"arm51",
        "arm52",
        "arm53",
};

static struct gpio_chip pc3x3_arm_gpio = {
	.label		    = "armgpio",
	.request	    = armgpio_request,
	.free		    = armgpio_free,
	.direction_input    = armgpio_direction_input,
	.direction_output   = armgpio_direction_output,
	.get		    = armgpio_get,
	.set		    = armgpio_set,
	.base		    = PC3X3_GPIO_PIN_ARM_0,
	.ngpio		    = ARRAY_SIZE(pc3x3_armgpio_pins),
	.names		    = pc3x3_armgpio_pins,
};
#endif /* CONFIG_PICOCHIP_PC3X3 */

/* The base address of SD-GPIO config registers in the AXI2Pico. */
#define PC302_GPIO_SD_PIN_CONFIG_BASE		0x9800
/* The base address of SD-GPIO analogue value registers in the AXI2Pico. */
#define PC302_GPIO_SD_PIN_ANALOGUE_VALUE_BASE	0x9801
/* The base address of SD-GPIO analogue rate registers in the AXI2Pico. */
#define PC302_GPIO_SD_PIN_ANALOGUE_RATE_BASE	0x9802
/* The address of the control value register in the AXI2Pico. */
#define PC302_GPIO_SD_CONTROL_VAL_REG		0x9882
/* The address of the control value high register in the AXI2Pico (pc3x3). */
#define PC302_GPIO_SD_CONTROL_VAL_HI_REG	0x9883
/* The address of the output value register in the AXI2Pico. */
#define PC302_GPIO_SD_OUTPUT_VAL_REG		0x9884
/* The address of the output value high register in the AXI2Pico (pc3x3). */
#define PC302_GPIO_SD_OUTPUT_HI_VAL_REG		0x9885
/* The address of the input value register in the AXI2Pico. */
#define PC302_GPIO_SD_INPUT_VAL_REG		0x9880
/* The address of the input value high register in the AXI2Pico (pc3x3). */
#define PC302_GPIO_SD_INPUT_VAL_HI_REG		0x9880
/* The address of the sleep register in the AXI2Pico. */
#define PC302_AXI2PICO_SLEEP_REG		0xA060
/* The spacing between SD-GPIO config registers. */
#define PC302_GPIO_SD_PIN_CONFIG_SPACING	4
/* Control source bit. */
#define PC302_GPIO_SD_CONFIG_CS_MASK		~(1 << 15)
/* Analogue not digital bit. */
#define PC302_GPIO_SD_CONFIG_AND		(1 << 14)
/* The mask for analogue converter size in the config register. */
#define PC302_GPIO_SD_CONV_SZ_MASK		0xF
/* Soft reset lock bit. */
#define PC302_GPIO_SD_CONFIG_SR_LOCK		(1 << 13)
/* PC302 AXI2Pico CAEID. */
#define PC302_AXI2PICO_CAEID			(0x9000)

/*
 * Get the address of a config register for a SD-GPIO pin.
 *
 * @_n The SD-GPIO pin number.
 *
 * Returns the base address of the register.
 */
#define PC302_GPIO_SD_PIN_CONFIG(_n) \
    PC302_GPIO_SD_PIN_CONFIG_BASE + ((_n) * PC302_GPIO_SD_PIN_CONFIG_SPACING)

/*
 * Get the address of a analogue rate register for a SD-GPIO pin.
 *
 * @_n The SD-GPIO pin number.
 *
 * Returns the base address of the register.
 */
#define PC302_GPIO_SD_PIN_ANALOGUE_RATE(_n) \
    PC302_GPIO_SD_PIN_ANALOGUE_RATE_BASE + \
        ((_n) * PC302_GPIO_SD_PIN_CONFIG_SPACING)

/*
 * Get the address of a analogue value register for a SD-GPIO pin.
 *
 * @_n The SD-GPIO pin number.
 *
 * Returns the base address of the register.
 */
#define PC302_GPIO_SD_PIN_ANALOGUE_VAL(_n) \
    PC302_GPIO_SD_PIN_ANALOGUE_VALUE_BASE + \
        ((_n) * PC302_GPIO_SD_PIN_CONFIG_SPACING)

static int
sdgpio_reset_config(unsigned block_pin,
		    int value)
{
	int ret;
	u16 data;

	ret = axi2cfg_config_read(PC302_AXI2PICO_CAEID,
				  PC302_GPIO_SD_PIN_CONFIG(block_pin),
				  &data, 1);
	if (1 != ret) {
		printf("failed to read config register for SDGPIO pin %u\n",
		       block_pin );
		return -EIO;
	}

	if (value)
		data |= PC302_GPIO_SD_CONFIG_SR_LOCK;
	else
		data &= ~PC302_GPIO_SD_CONFIG_SR_LOCK;
	ret = axi2cfg_config_write(PC302_AXI2PICO_CAEID,
				   PC302_GPIO_SD_PIN_CONFIG(block_pin),
				   &data, 1);
	if (1 != ret) {
		printf("failed to write config register for SDGPIO pin %u\n",
		       block_pin );
		return -EIO;
	}

	return 0;
}

static inline int
sdgpio_block_nr(unsigned gpio_nr)
{
	if (is_pc3x3()) {
		return gpio_nr - PC3X3_GPIO_PIN_SDGPIO_0;
	} else {
		if (gpio_nr >= PC302_GPIO_PIN_SDGPIO_0 &&
		    gpio_nr < PC302_GPIO_PIN_SDGPIO_7)
			return gpio_nr - PC302_GPIO_PIN_SDGPIO_0;
		else
			return (gpio_nr - PC302_GPIO_PIN_SDGPIO_8) + 8;
	}
}

static int
sdgpio_request(struct gpio_chip *chip,
	       unsigned offset)
{
	unsigned block_pin = sdgpio_block_nr(chip->base + offset);
	enum mux_setting mux;

	if (test_and_set_bit(offset + chip->base, pin_status))
		return -EBUSY;

	if (sdgpio_reset_config(block_pin, 1)) {
		test_and_clear_bit(offset + chip->base, pin_status);
		return -EIO;
	}

	/* Check the pin has been correctly multiplexed. */
	mux = pc3xx_get_pin_mux(offset + chip->base);
	if (!(mux & (MUX_SD | MUX_UNMUXED))) {
		/* The pin has an inconsistent mux setting. */
		printf("attempt to request sdgpio%u which is not correctly multiplexed\n",
			   block_pin);
		test_and_clear_bit(offset + chip->base, pin_status);
		return -EIO;
	}

	return 0;
}

static void
sdgpio_free(struct gpio_chip *chip,
	    unsigned offset)
{
	test_and_clear_bit(offset + chip->base, pin_status);
	pc3xx_gpio_configure_dac(chip->base + offset, 0, 0);
}

/*
 * Create a map of which pins are analogue and not digital. We have a separate
 * function for configuring pins as analogue. When we set analogue pins, we
 * don't treat the int parameter as a boolean anymore.
 */
DECLARE_BITMAP(a_not_d_map, ARCH_NR_GPIOS);

static int
sdgpio_get_digital_out_status(u32 *v)
{
	u16 data[2] = { 0, 0 };

	if (1 != axi2cfg_config_read(PC302_AXI2PICO_CAEID,
				PC302_GPIO_SD_OUTPUT_VAL_REG, &data[0], 1))
		return -EIO;

	if (is_pc3x3()) {
		if (1 != axi2cfg_config_read(PC302_AXI2PICO_CAEID,
					PC302_GPIO_SD_OUTPUT_HI_VAL_REG,
					&data[1], 1))
			return -EIO;
	}

	*v = data[0] | (data[1] << 16);

	return 0;
}

static int
sdgpio_set_digital_out_status(u32 v)
{
	u16 data[2] = { (u16)(v & 0xFFFF), (u16)((v >> 16) & 0xFFFF) };

	if (1 != axi2cfg_config_write(PC302_AXI2PICO_CAEID,
				PC302_GPIO_SD_OUTPUT_VAL_REG, &data[0], 1))
		return -EIO;

	if (is_pc3x3()) {
		if (1 != axi2cfg_config_write(PC302_AXI2PICO_CAEID,
					PC302_GPIO_SD_OUTPUT_HI_VAL_REG,
					&data[1], 1))
			return -EIO;
	}

	return 0;
}

static void
sdgpio_set(struct gpio_chip *chip,
	   unsigned offset,
	   int value)
{
	int ret;
	u16 data;
	unsigned block_pin = sdgpio_block_nr(chip->base + offset);

	if (!test_bit(chip->base + offset, a_not_d_map)) {
		u32 status;

		if (sdgpio_get_digital_out_status(&status)) {
			printf("failed to read SDGPIO output value reg\n");
			return;
		}

		status &= ~(1 << block_pin);
		status |= (!!value) << block_pin;

		if (sdgpio_set_digital_out_status(status)) {
			printf("failed to output control register for SDGPIO pin %u\n",
			       block_pin);
			return;
		}
	} else {
		/* Analogue mode */
		data = (u16)value;
		ret = axi2cfg_config_write(PC302_AXI2PICO_CAEID,
				PC302_GPIO_SD_PIN_ANALOGUE_VAL(block_pin),
				&data, 1);
		if (1 != ret) {
			printf("failed to write analogue value register for SDGPIO pin %u\n",
			       block_pin);
			return;
		}
	}
}

static int
sdgpio_get_digital_in_status(u32 *v)
{
	u16 data[2] = { 0, 0 };

	if (1 != axi2cfg_config_read(PC302_AXI2PICO_CAEID,
				PC302_GPIO_SD_INPUT_VAL_REG, &data[0], 1))
		return -EIO;

	if (is_pc3x3()) {
		if (1 != axi2cfg_config_read(PC302_AXI2PICO_CAEID,
					PC302_GPIO_SD_INPUT_VAL_HI_REG,
					&data[1], 1))
			return -EIO;
	}

	*v = data[0] | (data[1] << 16);

	return 0;
}

static int
sdgpio_get(struct gpio_chip *chip,
	   unsigned offset)
{
	int ret;
	u16 data;
	unsigned block_pin = sdgpio_block_nr(chip->base + offset);

	if (!test_bit(chip->base + offset, a_not_d_map)) {
		u32 status;

		if (sdgpio_get_digital_in_status(&status))
			return -EIO;

		return !!(status & (1 << block_pin));
	} else {
		/* Analogue mode */
		ret = axi2cfg_config_read(PC302_AXI2PICO_CAEID,
				PC302_GPIO_SD_PIN_ANALOGUE_VAL(block_pin),
				&data, 1);
		if (1 != ret) {
			printf("failed to read the analogue value register for SDGPIO pin %u\n",
			       block_pin);
			return -EIO;
		}

		return (int)data;
	}
}

static int
sdgpio_set_direction(unsigned block_pin,
		     int input)
{
	int ret;
	u16 data;

	ret = axi2cfg_config_read(PC302_AXI2PICO_CAEID,
			PC302_GPIO_SD_PIN_CONFIG(block_pin), &data, 1);
	if (1 != ret) {
		printf("failed to read config register for SDGPIO pin %u\n",
		       block_pin);
		return -EIO;
	}

	data &= PC302_GPIO_SD_CONFIG_CS_MASK;
	ret = axi2cfg_config_write(PC302_AXI2PICO_CAEID,
			PC302_GPIO_SD_PIN_CONFIG(block_pin), &data, 1);
	if (1 != ret) {
		printf("failed to write config register for SDGPIO pin %u\n",
		       block_pin );
		return -EIO;
	}

	/* Configure the pin to drive or not drive the output as appropriate. */
	ret = axi2cfg_config_read(PC302_AXI2PICO_CAEID,
			PC302_GPIO_SD_CONTROL_VAL_REG, &data, 1);
	if (1 != ret) {
		printf("failed to read SDGPIO control value register\n");
		return -EIO;
	}

	if (input)
		data &= ~(1 << block_pin);
	else
		data |= (1 << block_pin);

	ret = axi2cfg_config_write(PC302_AXI2PICO_CAEID,
			PC302_GPIO_SD_CONTROL_VAL_REG, &data, 1);
	if (1 != ret) {
		printf("failed to write control value register for SDGPIO pin %u\n",
		       block_pin);
		return -EIO;
	}

	return 0;
}

static int
sdgpio_direction_output(struct gpio_chip *chip,
			unsigned offset,
			int value)
{
	unsigned block_pin = sdgpio_block_nr(chip->base + offset);
	int ret = sdgpio_set_direction(block_pin, 0);

	if (ret)
		return ret;

	sdgpio_set(chip, offset, value);

	return 0;
}

static int
sdgpio_direction_input(struct gpio_chip *chip,
		       unsigned offset)
{
	unsigned block_pin = sdgpio_block_nr(chip->base + offset);

	return sdgpio_set_direction(block_pin, 1);
}

int
pc3xx_gpio_configure_dac(unsigned gpio,
                         u8 converter_size,
                         u16 analogue_rate)
{
	int ret;
	u16 data;
	unsigned block_pin = sdgpio_block_nr(gpio);

	ret = axi2cfg_config_read(PC302_AXI2PICO_CAEID,
			PC302_GPIO_SD_PIN_CONFIG(block_pin), &data, 1);
	if (1 != ret) {
		printf("failed to read config register for SDGPIO pin %u\n",
		       block_pin);
		return -EIO;
	}

	data &= PC302_GPIO_SD_CONFIG_CS_MASK;
	data &= ~PC302_GPIO_SD_CONV_SZ_MASK;
	if (!analogue_rate && !converter_size)
		data &= ~PC302_GPIO_SD_CONFIG_AND;
	else
		data |= PC302_GPIO_SD_CONFIG_AND;
	data |= (converter_size & PC302_GPIO_SD_CONV_SZ_MASK);

	ret = axi2cfg_config_write(PC302_AXI2PICO_CAEID,
			PC302_GPIO_SD_PIN_CONFIG(block_pin), &data, 1);
	if (1 != ret) {
		printf("failed to write config register for SDGPIO pin %u\n",
		       block_pin);
		return -EIO;
	}

	/* Configure the pin to drive the output. */
	ret = axi2cfg_config_read(PC302_AXI2PICO_CAEID,
			PC302_GPIO_SD_CONTROL_VAL_REG, &data, 1);
	if (1 != ret) {
		printf("failed to read SDGPIO control value register\n");
		return -EIO;
	}

	data |= (1 << block_pin);

	ret = axi2cfg_config_write(PC302_AXI2PICO_CAEID,
			PC302_GPIO_SD_CONTROL_VAL_REG, &data, 1);
	if (1 != ret) {
		printf("failed to write control value register for SDGPIO pin %u\n",
		       block_pin);
		return -EIO;
	}

	/* Write the analogue rate register */
	data = analogue_rate;
	ret = axi2cfg_config_write(PC302_AXI2PICO_CAEID,
			PC302_GPIO_SD_PIN_ANALOGUE_RATE(block_pin), &data, 1);
	if (1 != ret) {
		printf("failed to write analogue rate register for SDGPIO pin %u\n",
		       block_pin);
		return -EIO;
	}

	if (analogue_rate || converter_size)
		test_and_set_bit(gpio, a_not_d_map);
	else
		test_and_clear_bit(gpio, a_not_d_map);

	return 0;
}

/*
 * PC3X2 SD-GPIO.
 */
#ifdef CONFIG_PICOCHIP_PC3X2
char *sdgpio_lo_pins[] = {
	"sdgpio0",
	"sdgpio1",
	"sdgpio2",
	"sdgpio3",
	"sdgpio4",
	"sdgpio5",
	"sdgpio6",
	"sdgpio7",
};

static struct gpio_chip pc3x2_sd_gpio_lo = {
	.label		    = "sdpio_lo",
	.request	    = sdgpio_request,
	.free		    = sdgpio_free,
	.direction_input    = sdgpio_direction_input,
	.direction_output   = sdgpio_direction_output,
	.get		    = sdgpio_get,
	.set		    = sdgpio_set,
	.ngpio		    = ARRAY_SIZE(sdgpio_lo_pins),
	.base		    = PC302_GPIO_PIN_SDGPIO_0,
	.names		    = sdgpio_lo_pins,
};

char *sdgpio_shared_pins[] = {
	"sdgpio8",
	"sdgpio9",
	"sdgpio10",
	"sdgpio11",
	"sdgpio12",
	"sdgpio13",
	"sdgpio14",
	"sdgpio15",
};

static struct gpio_chip pc3x2_sd_gpio_shared = {
	.label		    = "sdgpio_shared",
	.request	    = sdgpio_request,
	.free		    = sdgpio_free,
	.direction_input    = sdgpio_direction_input,
	.direction_output   = sdgpio_direction_output,
	.get		    = sdgpio_get,
	.set		    = sdgpio_set,
	.ngpio		    = ARRAY_SIZE(sdgpio_shared_pins),
	.base		    = PC302_GPIO_PIN_SDGPIO_8,
	.names		    = sdgpio_shared_pins,
};
#endif /* CONFIG_PICOCHIP_PC3X2 */

/*
 * PC3X3 SD-GPIO.
 */
#ifdef CONFIG_PICOCHIP_PC3X3
char *pc3x3_sdgpio_pins[] = {
	"sdgpio0",
	"sdgpio1",
	"sdgpio2",
	"sdgpio3",
	"sdgpio4",
	"sdgpio5",
	"sdgpio6",
	"sdgpio7",
	"sdgpio8",
	"sdgpio9",
	"sdgpio10",
	"sdgpio11",
	"sdgpio12",
	"sdgpio13",
	"sdgpio14",
	"sdgpio15",
	"sdgpio16",
	"sdgpio17",
	"sdgpio18",
	"sdgpio19",
	"sdgpio20",
	"sdgpio21",
	"sdgpio22",
	"sdgpio23",
};

static struct gpio_chip pc3x3_sd_gpio = {
	.label		    = "sdgpio",
	.request	    = sdgpio_request,
	.free		    = sdgpio_free,
	.direction_input    = sdgpio_direction_input,
	.direction_output   = sdgpio_direction_output,
	.get		    = sdgpio_get,
	.set		    = sdgpio_set,
	.ngpio		    = ARRAY_SIZE(pc3x3_sdgpio_pins),
	.base		    = PC3X3_GPIO_PIN_SDGPIO_0,
	.names		    = pc3x3_sdgpio_pins,
};
#endif /* CONFIG_PICOCHIP_PC3X3 */

static struct gpio_chip *pc3x2_chips[] = {
#ifdef CONFIG_PICOCHIP_PC3X2
	&pc3x2_arm_gpio_lo,
	&pc3x2_arm_gpio_shared,
	&pc3x2_sd_gpio_lo,
	&pc3x2_sd_gpio_shared,
#endif /* CONFIG_PICOCHIP_PC3X2 */
};

static struct gpio_chip *pc3x3_chips[] = {
#ifdef CONFIG_PICOCHIP_PC3X3
	&pc3x3_arm_gpio,
	&pc3x3_sd_gpio,
#endif /* CONFIG_PICOCHIP_PC3X3 */
};

/* Public API -------------------------------------------------------------- */

int
pc3xx_gpio_init(void)
{
	int num_chips, i, ret = 0, id;
	u16 data = 0;
	struct gpio_chip **all_chips, *chip;

	/*
	 * Make sure that the AXI2Pico is awake for the SDGPIO transactions.
	 */
	if (1 != axi2cfg_config_write(PC302_AXI2PICO_CAEID,
				      PC302_AXI2PICO_SLEEP_REG, &data, 1)) {
		printf("unable to wake axi2pico\n");
		return -EIO;
	}

	if (is_pc3x3()) {
		num_chips = ARRAY_SIZE(pc3x3_chips);
		all_chips = pc3x3_chips;
	} else {
		num_chips = ARRAY_SIZE(pc3x2_chips);
		all_chips = pc3x2_chips;
	}

        /* Populate the gpio_desc[] array */
	for (chip = all_chips[0], i = 0; i < num_chips; ++i, ++chip) {
                int base = chip->base;
                for (id = base; id < base + chip->ngpio; id++) {
			gpio_desc[id].chip = chip;
		}
	}

	return ret;
}

int
pc3xx_gpio_request(unsigned gpio)
{
    	struct gpio_desc	*desc;
	struct gpio_chip	*chip;
        int			status = -EINVAL;

        desc = &gpio_desc[gpio];
	chip = desc->chip;

        if (chip == NULL)
		goto done;

        if (chip->request) {
		status = chip->request(chip, gpio - chip->base);
	}
done:
        return status;
}

void
pc3xx_gpio_free(unsigned gpio)
{
	struct gpio_desc	*desc;
	struct gpio_chip	*chip;

        desc = &gpio_desc[gpio];
	chip = desc->chip;

        if (chip->free) {
		chip->free(chip, gpio - chip->base);
	}
}

int
pc3xx_gpio_direction_input(unsigned gpio)
{
    	struct gpio_chip	*chip;
	struct gpio_desc	*desc = &gpio_desc[gpio];
	int			status = -EINVAL;
        chip = desc->chip;
	if (!chip || !chip->get || !chip->direction_input)
		goto fail;
	gpio -= chip->base;
	if (gpio >= chip->ngpio)
		goto fail;

        status = chip->direction_input(chip, gpio);

fail:
        return status;
}

int
pc3xx_gpio_direction_output(unsigned gpio,
                            int value)

{
        struct gpio_chip	*chip;
	struct gpio_desc	*desc = &gpio_desc[gpio];
	int			status = -EINVAL;

        chip = desc->chip;
	if (!chip || !chip->set || !chip->direction_output)
		goto fail;
	gpio -= chip->base;
	if (gpio >= chip->ngpio)
		goto fail;

        status = chip->direction_output(chip, gpio, value);

fail:
        return status;
}

void
pc3xx_gpio_set_value(unsigned gpio,
                     int value)

{
    	struct gpio_chip	*chip;
	struct gpio_desc	*desc = &gpio_desc[gpio];

        chip = desc->chip;

        chip->set(chip, gpio - chip->base, value);
}

int
pc3xx_gpio_get_value(unsigned gpio)
{
    	struct gpio_chip	*chip;
	struct gpio_desc	*desc = &gpio_desc[gpio];

        chip = desc->chip;

        return chip->get ? chip->get(chip, gpio - chip->base) : 0;
}
