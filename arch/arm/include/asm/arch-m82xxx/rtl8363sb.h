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


#ifndef _RTL8363SB_PHY_H
#define _RTL8363SB_PHY_H

//
// Stuff for portation between Diagnostics, U-boot and Linux driver software
//
#ifndef uint32_t
#define uint32_t unsigned int
#endif

#ifndef uint16_t
#define uint16_t unsigned short
#endif

#ifndef uint8_t
#define uint8_t  unsigned char
#endif

#ifndef int8_t
#define int8_t signed char
#endif

#ifndef int16_t
#define int16_t signed short
#endif

#ifndef int32_t
#define int32_t signed int
#endif

#ifndef BOOL
#define BOOL    int
#endif

/*add feature define here*/
/* dump MIB counters */
#define RTL8363SB_DUMP_MIB

/*
 * Exported functions
 */

unsigned short rtl8363sbReadRegister    (struct mii_bus * bus,
                                         unsigned short   addr
                                        );

void           rtl8363sbWriteRegister   (struct mii_bus * bus,
                                         unsigned short   addr,
                                         unsigned short   value
         );

unsigned short rtl8363sb_phy_debug_read (struct mii_device *  mdev,
                                         unsigned char        phy_ID,
                                         unsigned char        reg_addr
                                        );

void           rtl8363sb_phy_debug_write(struct mii_device * mdev,
                                         unsigned char       phy_ID,
                                         unsigned char       reg_addr,
                                         unsigned short      reg_val
                                        );

void   DumpRealTekStatistics   (struct mii_device * mdev, int MacPortNumber);

unsigned int CfgRealTekSwitch        (struct mii_bus *    bus);
unsigned int SetRealTekSwitchLoopback(struct mii_bus *    bus, int enabled);

#endif



