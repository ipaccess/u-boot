/*****************************************************************************
 * $picoChipHeaderSubst$
 *****************************************************************************/

/*!
* \file reset.S
* \brief Function used to reset the PC302 device.
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
#include <asm/arch/pc302.h>
#include <asm/arch/wdog.h>

/* Macros ------------------------------------------------------------------ */

/* Functions --------------------------------------------------------------- */

	.align	5

/*!
 *
 * Perform a software reset of the PC302 device.
 *
 */
.globl reset_cpu

reset_cpu:

        /* Use the 'fallback' watchdog method for reseting */

        ldr     r0, =PC302_WDOG_BASE

        /* Read the control register */
        ldr     r1, [r0, #WDOG_CONTROL_REG_OFFSET]

        /* Clear the 'Response mode' bit */
        bic     r1, #WDOGCONTROLREGRMODMASK

        /* Set the 'Watchdog Enable' bit */
        orr     r1, #WDOGCONTROLREGWDT_ENMASK

        /* Write to the control register */
        str     r1, [r0, #WDOG_CONTROL_REG_OFFSET]

        /* The Watchdog is hardwired for a 1 second timeout */

        /* 'Kick' the Watchdog into life */
        mov     r1, #WDOG_COUNTER_RESTART_KICK_VALUE
        str     r1, [r0, #WDOG_CONTROL_REG_OFFSET]

reset_loop_forever:

        /* We will never return from this function */
        b       reset_loop_forever
