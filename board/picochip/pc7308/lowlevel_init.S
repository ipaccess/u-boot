/*****************************************************************************
 * $picoChipHeaderSubst$
 *****************************************************************************/

/*!
* \file lowlevel_init.S
* \brief Various low level setup functions.
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
#include <config.h>
#include <asm/arch/picoxcell.h>
#include <asm/arch/gpio.h>
#include <asm/arch/axi2cfg.h>

/* Macros ------------------------------------------------------------------ */
/* This gives us a peripheral bus base address of 0x80000000
   and a memory region size of 4Mbytes */
#define PERIPHERAL_BUS_SETUP        0x8000000D

#define ASSERT_BOOT_ERROR_PIN       0
#define NEGATE_BOOT_ERROR_PIN       1
#define BOOT_ERROR_PIN_IS_OUTPUT    1

/* Prototypes--------------------------------------------------------------- */

/* Macros ------------------------------------------------------------------ */

/* Functions --------------------------------------------------------------- */

/*****************************************************************************
 *
 * lowlevel_init
 *
 * Description: Perform some low level PC3008 initialisation
 *
 *****************************************************************************/
 .global lowlevel_init

 lowlevel_init:

        /* U-Boot will have been loaded by a first stage bootloader,
         * so any low level initialisation will have happened in there
         */
        mov	pc, lr

/*****************************************************************************
 *
 * initialiseBootErrorPin
 *
 * Initialise the GPIO that is used as the Boot Error pin
 *
 *****************************************************************************/
 initialiseBootErrorPin:

        /* Get base address of the arm gpio block */
        ldr     r0,=PICOXCELL_GPIO_BASE

        /* Negate the Boot error pin (in the data register) */
        ldr     r1,[r0, #GPIO_SW_PORT_D_DR_REG_OFFSET]
        orr     r1,r1,#NEGATE_BOOT_ERROR_PIN
        str     r1,[r0, #GPIO_SW_PORT_D_DR_REG_OFFSET]

        /* Set up Boot Error pin to be an output */
        ldr     r1,[r0, #GPIO_SW_PORT_D_DDR_REG_OFFSET]
        orr     r1,r1,#BOOT_ERROR_PIN_IS_OUTPUT
        str     r1,[r0, #GPIO_SW_PORT_D_DDR_REG_OFFSET]

        /* Return to my caller */
        mov	pc, lr

/*****************************************************************************
 *
 * assertBootError
 *
 * Assert the Boot Error pin.
 *
 * Note: On the PC7308 platform this is used to illuminate an led.
 *
 *****************************************************************************/
 .global assertBootError

 assertBootError:

        /* Get base address of the arm gpio block */
        ldr     r0,=PICOXCELL_GPIO_BASE

        /* Assert the Boot Error pin */
        mov     r1, #ASSERT_BOOT_ERROR_PIN
        str     r1,[r0, #GPIO_SW_PORT_D_DR_REG_OFFSET]

        /* Return to my caller */
        mov	pc, lr
