/*****************************************************************************
 * $picoChipHeaderSubst$
 *****************************************************************************/

/*!
* \file timer.h
* \brief Definitions for the PC3xx Timer Block.
*
* Copyright (c) 2006-2011 Picochip Ltd
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
* All enquiries to support@picochip.com
*/

#ifndef __PC3XX_TIMER_H__
#define __PC3XX_TIMER_H__

/* Constants --------------------------------------------------------------- */

/*****************************************************************************/
/* Register Offset Addresses                                                 */
/*****************************************************************************/

/* Individual definitions */
#define TIMER1_LOAD_COUNT_REG_OFFSET        (0x00)
#define TIMER1_CURRENT_VALUE_REG_OFFSET     (0x04)
#define TIMER1_CONTROL_REG_OFFSET           (0x08)
#define TIMER1_EOI_REG_OFFSET               (0x0c)
#define TIMER1_INT_STATUS_REG_OFFSET        (0x10)

#define TIMER2_LOAD_COUNT_REG_OFFSET        (0x14)
#define TIMER2_CURRENT_VALUE_REG_OFFSET     (0x18)
#define TIMER2_CONTROL_REG_OFFSET           (0x1c)
#define TIMER2_EOI_REG_OFFSET               (0x20)
#define TIMER2_INT_STATUS_REG_OFFSET        (0x24)

#define TIMER3_LOAD_COUNT_REG_OFFSET        (0x28)
#define TIMER3_CURRENT_VALUE_REG_OFFSET     (0x2c)
#define TIMER3_CONTROL_REG_OFFSET           (0x30)
#define TIMER3_EOI_REG_OFFSET               (0x34)
#define TIMER3_INT_STATUS_REG_OFFSET        (0x38)

#define TIMER4_LOAD_COUNT_REG_OFFSET        (0x3c)
#define TIMER4_CURRENT_VALUE_REG_OFFSET     (0x40)
#define TIMER4_CONTROL_REG_OFFSET           (0x44)
#define TIMER4_EOI_REG_OFFSET               (0x48)
#define TIMER4_INT_STATUS_REG_OFFSET        (0x4c)

#define TIMERS_INT_STATUS_REG_OFFSET        (0xa0)
#define TIMERS_EOI_REG_OFFSET               (0xa4)
#define TIMERS_RAW_INT_STATUS_REG_OFFSET    (0xa8)
#define TIMERS_COMP_VERSION_REG_OFFSET      (0xac)

/* Macros ------------------------------------------------------------------ */

/* Generic definitions 'borrowed' from U-Boot land */

/* The number of timers in the hardware, numbered 0 to N-1 */
#define TIMERNUMBEROFTIMERS                 (4)

/* Register definitions for the timers */
#define TIMERNLOADCOUNTREGOFFSET(__N)       (0x0000 + (0x14 * (__N)))
#define TIMERNCURRENTVALUEREGOFFSET(__N)    (0x0004 + (0x14 * (__N)))
#define TIMERNCONTROLREGOFFSET(__N)         (0x0008 + (0x14 * (__N)))
#define TIMERNEOIREGOFFSET(__N)             (0x000c + (0x14 * (__N)))
#define TIMERNINTERRUPTSTATUSREGOFFSET(__N) (0x0010 + (0x14 * (__N)))

/* Timer N control register bit definitions */
#define TIMERENABLE                         (0x00000001)
#define TIMERMODE                           (0x00000002)
#define TIMERINTERRUPTMASK                  (0x00000004)


/* Register definitions for global timer registers */
#define TIMERSINTERRUPTSTATUSREGOFFSET      (0x00a0)
#define TIMERSEOIREGOFFSET                  (0x00a4)
#define TIMERSRAWINTERRUPTSTATUSREGOFFSET   (0x00a8)

/* Global Timer Registers bit definitions */
#define TIMER(__N)                          (0x00000001 << (__N))

#endif /* __PC3XX_TIMER_H__ */
