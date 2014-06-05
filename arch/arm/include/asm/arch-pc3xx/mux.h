/*****************************************************************************
 * $picoChipHeaderSubst$
 *****************************************************************************/

/*!
* \file mux.h
* \brief Definitions for the PC3xx gpio muxing
*
* Copyright (c) 2006-2011 Picochip Ltd
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
* All enquiries to support@picochip.com
*/

#ifndef __PC3XX_MUX_H__
#define __PC3XX_MUX_H__

/* stringify used in mux.c */
#define __stringify_1(x...)     #x
#define __stringify(x...)       __stringify_1(x)

/*
 * Pins can typically be:
 *	- a system function such as EBI, SSI etc,
 *	- ARM controlled GPIO.
 *	- picoArray controlled GPIO.
 *	- not multiplexed at all (MUX_UNMUXED).
 */
enum mux_setting {
	MUX_PERIPHERAL	= (1 << 0),
	MUX_ARM		= (1 << 1),
	MUX_SD		= (1 << 2),
	MUX_UNMUXED	= (1 << 3),
};

int
pc3xx_pin_set_mux(int pin_nr,
		  enum mux_setting setting);

int
pc3xx_group_set_mux(const char *group_name,
		    enum mux_setting setting);

int
pc3xx_get_pin_mux(int pin_nr);

void
pc3xx_muxing_init(void);

#endif /* __PC3XX_MUX_H__ */
