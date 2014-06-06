/*****************************************************************************
 * $picoChipHeaderSubst$
 *****************************************************************************/

/*!
* \file clk.h
* \brief Definitions for the picoxcell clock related functions
*
* Copyright (c) 2006-2011 Picochip Ltd
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
* All enquiries to support@picochip.com
*/
#include <common.h>

#ifndef __PICOXCELL_CLK_H__
#define __PICOXCELL_CLK_H__

static inline unsigned long get_macb_pclk_rate(unsigned int dev_id)
{
	return PICOXCELL_AHB_CLOCK_FREQ;
}

#endif /* __PICOXCELL_CLK_H__ */
