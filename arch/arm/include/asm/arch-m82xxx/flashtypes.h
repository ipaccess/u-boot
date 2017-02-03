/*
 * Copyright(c) 2007-2014 Intel Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program; if not, write to the Free Software 
 * Foundation, Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 * The full GNU General Public License is included in this distribution 
 * in the file called LICENSE.GPL.
 *
 * Contact Information:
 * Intel Corporation
 */

#ifndef __FLASHTYPES_H__
#define __FLASHTYPES_H__

#include "systypes.h"

#define DEBUG(...)

typedef struct tagFLASH_DEVICE
{
	UINT32 	baseAddress;
	UINT32	MfgID;
	UINT32	DeviceID;
	UINT32	BlockSize;
	UINT32	TotalSize;

	UINT32 (*read )(struct tagFLASH_DEVICE* pFlash, UINT8* buf, UINT32 addr, UINT32 len);
	UINT32 (*write)(struct tagFLASH_DEVICE* pFlash, UINT8* buf, UINT32 addr, UINT32 len);
	UINT32 (*erase)(struct tagFLASH_DEVICE* pFlash, UINT32 addr, UINT32 sec_cnt);
    
} FLASH_DEVICE, *PFLASH_DEVICE;

/**
	@brief The init data of flash device driver 
	@note  In a future will be expanded by new fields
*/
typedef struct 
{
	UINT32 	nBaseAddress;				/**< The base address of device */
    UINT32  nCfg;

}FLASH_DRV_INIT, *PFLASH_DRV_INIT;

#endif /*__FLASHTYPES_H__ */

