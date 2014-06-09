/*****************************************************************************
 * $picoChipHeaderSubst$
 *****************************************************************************/

/*!
 * \file timer.c
 * \brief Useful functions for timer implementation.
 *
 * Copyright (c) 2006-2011 Picochip Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * All enquiries to support@picochip.com
 */

/*
 * (C) Copyright 2003
 * Texas Instruments <www.ti.com>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Alex Zuepke <azu@sysgo.de>
 *
 * (C) Copyright 2002-2004
 * Gary Jennejohn, DENX Software Engineering, <gj@denx.de>
 *
 * (C) Copyright 2004
 * Philippe Robin, ARM Ltd. <philippe.robin@arm.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/arch/utilities.h>
#include <asm/arch/timer.h>
#include <div64.h>

/*
 * The timer is a decrementer, it runs at a frequency of PICOXCELL_AHB_CLOCK_FREQ
 */
#define READ_TIMER() (picoxcell_read_register (CONFIG_SYS_TIMERBASE +\
                                               TIMERNCURRENTVALUEREGOFFSET(0)))

#define TIMER_LOAD_VAL 0xFFFFFFFF

static unsigned long long timestamp;
static unsigned long lastdec;

void reset_timer_masked (void)
{
	lastdec = READ_TIMER ();
	timestamp = 0;
}

/* Timer init function */
int timer_init (void)
{
	reset_timer_masked ();
	return 0;
}

/* Restart counting from 0 */
void reset_timer (void)
{
	reset_timer_masked ();
}

static inline unsigned long long tick_to_time (unsigned long long tick)
{
	tick *= CONFIG_SYS_HZ;
	do_div (tick, PICOXCELL_AHB_CLOCK_FREQ);
	return tick;
}

static inline unsigned long long time_to_tick (unsigned long long time)
{
	time *= PICOXCELL_AHB_CLOCK_FREQ;
	do_div (time, CONFIG_SYS_HZ);
	return time;
}

static inline unsigned long long us_to_tick (unsigned long long us)
{
	us = us * PICOXCELL_AHB_CLOCK_FREQ + 999999;
	do_div (us, 1000000);
	return us;
}

unsigned long long get_ticks (void)
{
	ulong now = READ_TIMER ();

	if (lastdec >= now)
		timestamp += (lastdec - now);
	else
		timestamp += (lastdec + TIMER_LOAD_VAL - now);

	lastdec = now;

	return timestamp;
}

ulong get_timer_masked (void)
{
	return tick_to_time (get_ticks ());
}

/* Return how many HZ passed since "base" */
ulong get_timer (ulong base)
{
	return get_timer_masked () - base;
}

/* Delay 'usec' micro seconds */
void __udelay (unsigned long usec)
{
	unsigned long long tmp;
	unsigned long long tmo;

	tmo = us_to_tick (usec);
	tmp = get_ticks () + tmo;

	while (get_ticks () < tmp) ;
}
