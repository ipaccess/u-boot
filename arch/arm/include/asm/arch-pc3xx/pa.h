/*****************************************************************************
 * $picoChipHeaderSubst$
 *****************************************************************************/

/*!
* \file pa.h
* \brief Definitions for the picoArray.
*
* Copyright (c) 2006-2011 Picochip Ltd
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
* All enquiries to support@picochip.com
*/

#ifndef __PC3XX_PA_H__
#define __PC3XX_PA_H__

/* Constants --------------------------------------------------------------- */

/* Broadcast Id for memif-Shared */
#define PA_BROADCAST_ID_MEMIF   (0x8020)

/* The memif-Shared aeid changed from PC3x2 devices to PC3x3 devices,
 * to avoid lots of code changes we just use the memif-Shared
 * broadcast address instead.
 */
#define PA_AEID_MEMIF       (PA_BROADCAST_ID_MEMIF)

#define PA_CONFIG_WRITE     (0x00010000)
#define PA_CONFIG_READ      (0x00020000)
#define PA_CONFIG_ADDR      (0x00040000)
#define PA_CONFIG_AEID      (0x00080000)
#define PA_CONFIG_VALID     (0x00010000)
#define PA_CONFIG_FAIL      (0x00020000)
#define PA_CONFIG_TIMEOUT   (0x00040000)

#endif /* __PC3XX_PA_H__ */
