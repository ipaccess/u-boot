/*****************************************************************************
 * $picoChipHeaderSubst$
 *****************************************************************************/

/*!
* \file pc3xxgpio.h
* \brief Definitions for use with the PC3xx gpio library
*
* Copyright (c) 2006-2011 Picochip Ltd
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
* All enquiries to support@picochip.com
*/

#ifndef __MACH_GPIO_H__
#define __MACH_GPIO_H__

/* Constants --------------------------------------------------------------- */

/* Use on PC3x2 devices */
enum {
	PC302_GPIO_PIN_ARM_0 =  0,  /* ARM GPIO pin identifiers. */
	PC302_GPIO_PIN_ARM_1,
	PC302_GPIO_PIN_ARM_2,
	PC302_GPIO_PIN_ARM_3,
	PC302_GPIO_PIN_ARM_4,
	PC302_GPIO_PIN_ARM_5,
	PC302_GPIO_PIN_ARM_6,
	PC302_GPIO_PIN_ARM_7,
	PC302_GPIO_PIN_SDGPIO_0,    /* SDGPIO pin identifiers. */
	PC302_GPIO_PIN_SDGPIO_1,
	PC302_GPIO_PIN_SDGPIO_2,
	PC302_GPIO_PIN_SDGPIO_3,
	PC302_GPIO_PIN_SDGPIO_4,
	PC302_GPIO_PIN_SDGPIO_5,
	PC302_GPIO_PIN_SDGPIO_6,
	PC302_GPIO_PIN_SDGPIO_7,
	PC302_GPIO_PIN_ARM_8,      /* ARM shared pins. */
	PC302_GPIO_PIN_ARM_9,
	PC302_GPIO_PIN_ARM_10,
	PC302_GPIO_PIN_ARM_11,
	PC302_GPIO_PIN_ARM_12,
	PC302_GPIO_PIN_ARM_13,
	PC302_GPIO_PIN_ARM_14,
	PC302_GPIO_PIN_ARM_15,
	PC302_GPIO_PIN_SDGPIO_8,  /* SDGPIO shared pins. */
	PC302_GPIO_PIN_SDGPIO_9,
	PC302_GPIO_PIN_SDGPIO_10,
	PC302_GPIO_PIN_SDGPIO_11,
	PC302_GPIO_PIN_SDGPIO_12,
	PC302_GPIO_PIN_SDGPIO_13,
	PC302_GPIO_PIN_SDGPIO_14,
	PC302_GPIO_PIN_SDGPIO_15,
};

/* Use on PC3x3 devices */
enum {
	PC3X3_GPIO_PIN_ARM_0,
	PC3X3_GPIO_PIN_ARM_1,
	PC3X3_GPIO_PIN_ARM_2,
	PC3X3_GPIO_PIN_ARM_3,
	PC3X3_GPIO_PIN_ARM_4,
	PC3X3_GPIO_PIN_ARM_5,
	PC3X3_GPIO_PIN_ARM_6,
	PC3X3_GPIO_PIN_ARM_7,
	PC3X3_GPIO_PIN_ARM_8,
	PC3X3_GPIO_PIN_ARM_9,
	PC3X3_GPIO_PIN_ARM_10,
	PC3X3_GPIO_PIN_ARM_11,
	PC3X3_GPIO_PIN_ARM_12,
	PC3X3_GPIO_PIN_ARM_13,
	PC3X3_GPIO_PIN_ARM_14,
	PC3X3_GPIO_PIN_ARM_15,
	PC3X3_GPIO_PIN_ARM_16,
	PC3X3_GPIO_PIN_ARM_17,
	PC3X3_GPIO_PIN_ARM_18,
	PC3X3_GPIO_PIN_ARM_19,
	PC3X3_GPIO_PIN_ARM_20,
	PC3X3_GPIO_PIN_ARM_21,
	PC3X3_GPIO_PIN_ARM_22,
	PC3X3_GPIO_PIN_ARM_23,
	PC3X3_GPIO_PIN_ARM_24,
	PC3X3_GPIO_PIN_ARM_25,
	PC3X3_GPIO_PIN_ARM_26,
	PC3X3_GPIO_PIN_ARM_27,
	PC3X3_GPIO_PIN_ARM_28,
	PC3X3_GPIO_PIN_ARM_29,
	PC3X3_GPIO_PIN_ARM_30,
	PC3X3_GPIO_PIN_ARM_31,
	PC3X3_GPIO_PIN_ARM_32,
	PC3X3_GPIO_PIN_ARM_33,
	PC3X3_GPIO_PIN_ARM_34,
	PC3X3_GPIO_PIN_ARM_35,
	PC3X3_GPIO_PIN_ARM_36,
	PC3X3_GPIO_PIN_ARM_37,
	PC3X3_GPIO_PIN_ARM_38,
	PC3X3_GPIO_PIN_ARM_39,
	PC3X3_GPIO_PIN_ARM_40,
	PC3X3_GPIO_PIN_ARM_41,
	PC3X3_GPIO_PIN_ARM_42,
	PC3X3_GPIO_PIN_ARM_43,
	PC3X3_GPIO_PIN_ARM_44,
	PC3X3_GPIO_PIN_ARM_45,
	PC3X3_GPIO_PIN_ARM_46,
	PC3X3_GPIO_PIN_ARM_47,
	PC3X3_GPIO_PIN_ARM_48,
	PC3X3_GPIO_PIN_ARM_49,
	PC3X3_GPIO_PIN_ARM_50,
	PC3X3_GPIO_PIN_ARM_51,
	PC3X3_GPIO_PIN_ARM_52,
	PC3X3_GPIO_PIN_ARM_53,
	PC3X3_GPIO_PIN_SDGPIO_0,
	PC3X3_GPIO_PIN_SDGPIO_1,
	PC3X3_GPIO_PIN_SDGPIO_2,
	PC3X3_GPIO_PIN_SDGPIO_3,
	PC3X3_GPIO_PIN_SDGPIO_4,
	PC3X3_GPIO_PIN_SDGPIO_5,
	PC3X3_GPIO_PIN_SDGPIO_6,
	PC3X3_GPIO_PIN_SDGPIO_7,
	PC3X3_GPIO_PIN_SDGPIO_8,
	PC3X3_GPIO_PIN_SDGPIO_9,
	PC3X3_GPIO_PIN_SDGPIO_10,
	PC3X3_GPIO_PIN_SDGPIO_11,
	PC3X3_GPIO_PIN_SDGPIO_12,
	PC3X3_GPIO_PIN_SDGPIO_13,
	PC3X3_GPIO_PIN_SDGPIO_14,
	PC3X3_GPIO_PIN_SDGPIO_15,
	PC3X3_GPIO_PIN_SDGPIO_16,
	PC3X3_GPIO_PIN_SDGPIO_17,
	PC3X3_GPIO_PIN_SDGPIO_18,
	PC3X3_GPIO_PIN_SDGPIO_19,
	PC3X3_GPIO_PIN_SDGPIO_20,
	PC3X3_GPIO_PIN_SDGPIO_21,
	PC3X3_GPIO_PIN_SDGPIO_22,
	PC3X3_GPIO_PIN_SDGPIO_23,
};

/* Prototypes--------------------------------------------------------------- */
/**
 * Initialise the gpio library for use
 *
 * \return Returns zero on success, non-zero on failure.
 */
int
pc3xx_gpio_init(void);

/**
 * Request a new GPIO pin. This implements part of the Linux GPIO guidelines.
 *
 * \param gpio The pin to request.
 * \return Returns zero on success, non-zero on failure.
 */
int
pc3xx_gpio_request(unsigned gpio);

/**
 * Free a GPIO pin previously requested with gpio_request().
 *
 * \param gpio The GPIO pin to free.
 */
void
pc3xx_gpio_free(unsigned gpio);

/**
 * Set the direction of a GPIO pin requested with gpio_request() to be an
 * input.
 *
 * \param gpio The GPIO pin to configure.
 * \return Returns zero on success, non-zero on failure.
 */
int
pc3xx_gpio_direction_input(unsigned gpio);

/**
 * Set the direction of a GPIO pin requested with gpio_request() to be an
 * output.
 *
 * \param gpio The GPIO pin to configure.
 * \param value The initial output value for the gpio pin.
 * \return Returns zero on success, non-zero on failure.
 */
int
pc3xx_gpio_direction_output(unsigned gpio,
                            int value);

/**
 * Set the value of a GPIO pin.
 *
 * \param gpio The number of the pin to set the value of.
 * \param value The value to set the pin to.
 */
void
pc3xx_gpio_set_value(unsigned gpio,
                     int value);

/**
 * Get the value of a GPIO pin.
 *
 * \param gpio The number of the pin to get the value of.
 * \return Returns the value of the pin on success,
 * negative on failure.
 */
int
pc3xx_gpio_get_value(unsigned gpio);

int
pc3xx_gpio_configure_dac(unsigned gpio,
                         u8 converter_size,
                         u16 analogue_rate);

#endif /* __MACH_GPIO_H__ */
