/*****************************************************************************
 * $picoChipHeaderSubst$
 *****************************************************************************/

/*!
* \file lowlevel_init.S
* \brief Various low level setup functions.
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
 * Description: Perform some low level PC3032 initialisation
 *
 *****************************************************************************/
 .global lowlevel_init

 lowlevel_init:
        /* The memory interface setup code uses the stack, so we
         * temporarily set one up.
         */
        ldr	sp, =(PICOXCELL_SRAM_BASE + PICOXCELL_SRAM_SIZE)

        /* Setup the Periheral Port Memory Remap Register */
        ldr     r0, =PERIPHERAL_BUS_SETUP
        mcr     p15, 0, r0, c15, c2, 4

        /* Now we need to remap the flash memory otherwsie we'll not be
         * able to talk to sdram later on.
         */
        orr     pc, pc, #PICOXCELL_FLASH_BASE
        nop
        nop

        /* We are now executing code from the 'correct' flash memory address.
         * Now we fix up the ip and the lr registers. This ensures that when
         * we return from here to the cpu_init_crit() and ultimately the
         * reset() we keep running code in the remapped address.
         */
        orr     ip, ip, #PICOXCELL_FLASH_BASE
        orr     lr, lr, #PICOXCELL_FLASH_BASE

        /* Perform the 'remap' in the axi2cfg block
         * This turns off the address aliasing in the hardware
         */
        ldr     r0, =PICOXCELL_AXI2CFG_BASE
        ldr     r1, [r0, #AXI2CFG_SYS_CONFIG_REG_OFFSET]
        bic     r1, #AXI2CFG_SYS_CONFIG_RW_REMAP_MASK
        str     r1, [r0, #AXI2CFG_SYS_CONFIG_REG_OFFSET]
        nop
        nop

        push    {r0-r12, r14}

        bl      initialiseBootErrorPin

        bl      pc30xx_memif_shared_init
        bl      pc30xx_check_memif_shared_setup

        bl      pc30xx_memif_arm_init

        pop     {r0-r12, r14}

        /* Return to my caller */
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
 * Note: On the PC73032 platform this is used to illuminate an led.
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
