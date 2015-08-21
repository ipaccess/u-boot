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

#if 1
/** @brief Define to turn on printing of all SMI (MDIO) operations for debug */
#define PRINT_RTL8363SB_OPERATIONS
int print_rtl8363sb_debug_print_enabled = 1;
#endif

#ifdef LINUX
//
// Original includes for linux code base
//
#include <common.h>
#include <init.h>
#include <miidev.h>

#else
//
// Includes for portation to Transcede device
//
//#include "syscore.h"
//#include "sysdata.h"
#include <common.h>
#include <asm/arch/mdio_bitbang.h> // Include for MDIO bit bang driver
#endif

#ifndef mii_device
/** @brief Map MDIO device structure from RealTek code to mii_bus structure */
#define mii_device mii_bus
#endif

/** @brief Map mdelay call to diagnostics SysDelay functoin */
#ifndef mdelay
#define mdelay SysDelay
#endif

#ifdef DEBUG_RTL8363SB
#ifndef DEBUG_PRINT
#ifdef LINUX
#define DEBUG_PRINT printk
#else
#define DEBUG_PRINT printf
#endif
#endif
#endif

#ifndef INFO_PRINT
#ifdef LINUX
#define INFO_PRINT printk
#else
#define INFO_PRINT printf
#endif
#endif

#ifndef TRUE
#define TRUE    1
#endif

#ifndef FALSE
#define FALSE   0
#endif

#ifndef uint32_t
#define uint32_t unsigned int
#endif

#ifndef uint16_t
#define uint16_t unsigned short
#endif

#ifndef uint8_t
#define uint8_t  unsigned char
#endif

#ifndef UINT32
#define UINT32 uint32_t
#endif

#ifndef UINT16
#define UINT16 uint16_t
#endif

#ifndef UINT8
#define UINT8 uint8_t
#endif

#ifndef SYSRC_SUCCESS
#define SYSRC_SUCCESS 0
#endif
//
// Include defines and types after all portability/portation
// defines and types defined (to allow for easier poration
// to U-boot and Linux)
//
#include <asm/arch/rtl8363sb.h>

const unsigned short rtl8362sbInitTable[] =
{
    (unsigned short) 0x13C2, (unsigned short) 0x0249,
    (unsigned short) 0x201F, (unsigned short) 0x0007,
    (unsigned short) 0x201E, (unsigned short) 0x002C,
    (unsigned short) 0x2019, (unsigned short) 0x0504,
    (unsigned short) 0x201F, (unsigned short) 0x0000,
    (unsigned short) 0x203F, (unsigned short) 0x0007,
    (unsigned short) 0x203E, (unsigned short) 0x002C,
    (unsigned short) 0x2039, (unsigned short) 0x0504,
    (unsigned short) 0x203F, (unsigned short) 0x0000,
    (unsigned short) 0x205F, (unsigned short) 0x0007,
    (unsigned short) 0x205E, (unsigned short) 0x002C,
    (unsigned short) 0x2059, (unsigned short) 0x0504,
    (unsigned short) 0x205F, (unsigned short) 0x0000,
    (unsigned short) 0x207F, (unsigned short) 0x0007,
    (unsigned short) 0x207E, (unsigned short) 0x002C,
    (unsigned short) 0x2079, (unsigned short) 0x0504,
    (unsigned short) 0x207F, (unsigned short) 0x0000,
    (unsigned short) 0x209F, (unsigned short) 0x0007,
    (unsigned short) 0x209E, (unsigned short) 0x002C,
    (unsigned short) 0x2099, (unsigned short) 0x0504,
    (unsigned short) 0x209F, (unsigned short) 0x0000,
    (unsigned short) 0x1303, (unsigned short) 0x0778,
    (unsigned short) 0x1304, (unsigned short) 0x7777,
    (unsigned short) 0x13E2, (unsigned short) 0x01FE,
    (unsigned short) 0x1310, (unsigned short) 0x1075,
    (unsigned short) 0x1305, (unsigned short) 0x0003,
    (unsigned short) 0x1B03, (unsigned short) 0x0876,
    (unsigned short) 0x1200, (unsigned short) 0x7FC4,
    (unsigned short) 0x1305, (unsigned short) 0xC000,
    (unsigned short) 0x121E, (unsigned short) 0x03CA,
    (unsigned short) 0x1233, (unsigned short) 0x0352,
    (unsigned short) 0x1234, (unsigned short) 0x0064,
    (unsigned short) 0x1237, (unsigned short) 0x0096,
    (unsigned short) 0x1238, (unsigned short) 0x0078,
    (unsigned short) 0x1239, (unsigned short) 0x0084,
    (unsigned short) 0x123A, (unsigned short) 0x0030,
    (unsigned short) 0x205F, (unsigned short) 0x0002,
    (unsigned short) 0x2059, (unsigned short) 0x1A00,
    (unsigned short) 0x205F, (unsigned short) 0x0000,
    (unsigned short) 0x207F, (unsigned short) 0x0002,
    (unsigned short) 0x2077, (unsigned short) 0x0000,
    (unsigned short) 0x2078, (unsigned short) 0x0000,
    (unsigned short) 0x2079, (unsigned short) 0x0000,
    (unsigned short) 0x207A, (unsigned short) 0x0000,
    (unsigned short) 0x207B, (unsigned short) 0x0000,
    (unsigned short) 0x207F, (unsigned short) 0x0000,
    (unsigned short) 0x205F, (unsigned short) 0x0002,
    (unsigned short) 0x2053, (unsigned short) 0x0000,
    (unsigned short) 0x2054, (unsigned short) 0x0000,
    (unsigned short) 0x2055, (unsigned short) 0x0000,
    (unsigned short) 0x2056, (unsigned short) 0x0000,
    (unsigned short) 0x2057, (unsigned short) 0x0000,
    (unsigned short) 0x205F, (unsigned short) 0x0000,
    (unsigned short) 0x133F, (unsigned short) 0x0030,
    (unsigned short) 0x133E, (unsigned short) 0x000E,
    (unsigned short) 0x221F, (unsigned short) 0x0005,
    (unsigned short) 0x2205, (unsigned short) 0x8B86,
    (unsigned short) 0x2206, (unsigned short) 0x800E,
    (unsigned short) 0x221F, (unsigned short) 0x0000,
    (unsigned short) 0x133F, (unsigned short) 0x0010,
    (unsigned short) 0x12A3, (unsigned short) 0x2200,
    (unsigned short) 0x6107, (unsigned short) 0xE58B,
    (unsigned short) 0x6103, (unsigned short) 0xA970,
    (unsigned short) 0x0018, (unsigned short) 0x0F00,
    (unsigned short) 0x0038, (unsigned short) 0x0F00,
    (unsigned short) 0x0058, (unsigned short) 0x0F00,
    (unsigned short) 0x0078, (unsigned short) 0x0F00,
    (unsigned short) 0x0098, (unsigned short) 0x0F00,
    (unsigned short) 0x133F, (unsigned short) 0x0030,
    (unsigned short) 0x133E, (unsigned short) 0x000E,
    (unsigned short) 0x221F, (unsigned short) 0x0005,
    (unsigned short) 0x2205, (unsigned short) 0x8B6E,
    (unsigned short) 0x2206, (unsigned short) 0x0000,
    (unsigned short) 0x220F, (unsigned short) 0x0100,
    (unsigned short) 0x2205, (unsigned short) 0xFFF6,
    (unsigned short) 0x2206, (unsigned short) 0x0080,
    (unsigned short) 0x2205, (unsigned short) 0x8000,
    (unsigned short) 0x2206, (unsigned short) 0x0280,
    (unsigned short) 0x2206, (unsigned short) 0x2BF7,
    (unsigned short) 0x2206, (unsigned short) 0x00E0,
    (unsigned short) 0x2206, (unsigned short) 0xFFF7,
    (unsigned short) 0x2206, (unsigned short) 0xA080,
    (unsigned short) 0x2206, (unsigned short) 0x02AE,
    (unsigned short) 0x2206, (unsigned short) 0xF602,
    (unsigned short) 0x2206, (unsigned short) 0x0153,
    (unsigned short) 0x2206, (unsigned short) 0x0201,
    (unsigned short) 0x2206, (unsigned short) 0x6602,
    (unsigned short) 0x2206, (unsigned short) 0x8044,
    (unsigned short) 0x2206, (unsigned short) 0x0201,
    (unsigned short) 0x2206, (unsigned short) 0x7CE0,
    (unsigned short) 0x2206, (unsigned short) 0x8B8C,
    (unsigned short) 0x2206, (unsigned short) 0xE18B,
    (unsigned short) 0x2206, (unsigned short) 0x8D1E,
    (unsigned short) 0x2206, (unsigned short) 0x01E1,
    (unsigned short) 0x2206, (unsigned short) 0x8B8E,
    (unsigned short) 0x2206, (unsigned short) 0x1E01,
    (unsigned short) 0x2206, (unsigned short) 0xA000,
    (unsigned short) 0x2206, (unsigned short) 0xE4AE,
    (unsigned short) 0x2206, (unsigned short) 0xD8EE,
    (unsigned short) 0x2206, (unsigned short) 0x85C0,
    (unsigned short) 0x2206, (unsigned short) 0x00EE,
    (unsigned short) 0x2206, (unsigned short) 0x85C1,
    (unsigned short) 0x2206, (unsigned short) 0x00EE,
    (unsigned short) 0x2206, (unsigned short) 0x8AFC,
    (unsigned short) 0x2206, (unsigned short) 0x07EE,
    (unsigned short) 0x2206, (unsigned short) 0x8AFD,
    (unsigned short) 0x2206, (unsigned short) 0x73EE,
    (unsigned short) 0x2206, (unsigned short) 0xFFF6,
    (unsigned short) 0x2206, (unsigned short) 0x00EE,
    (unsigned short) 0x2206, (unsigned short) 0xFFF7,
    (unsigned short) 0x2206, (unsigned short) 0xFC04,
    (unsigned short) 0x2206, (unsigned short) 0xF8E0,
    (unsigned short) 0x2206, (unsigned short) 0x8B8E,
    (unsigned short) 0x2206, (unsigned short) 0xAD20,
    (unsigned short) 0x2206, (unsigned short) 0x0302,
    (unsigned short) 0x2206, (unsigned short) 0x8050,
    (unsigned short) 0x2206, (unsigned short) 0xFC04,
    (unsigned short) 0x2206, (unsigned short) 0xF8F9,
    (unsigned short) 0x2206, (unsigned short) 0xE08B,
    (unsigned short) 0x2206, (unsigned short) 0x85AD,
    (unsigned short) 0x2206, (unsigned short) 0x2548,
    (unsigned short) 0x2206, (unsigned short) 0xE08A,
    (unsigned short) 0x2206, (unsigned short) 0xE4E1,
    (unsigned short) 0x2206, (unsigned short) 0x8AE5,
    (unsigned short) 0x2206, (unsigned short) 0x7C00,
    (unsigned short) 0x2206, (unsigned short) 0x009E,
    (unsigned short) 0x2206, (unsigned short) 0x35EE,
    (unsigned short) 0x2206, (unsigned short) 0x8AE4,
    (unsigned short) 0x2206, (unsigned short) 0x00EE,
    (unsigned short) 0x2206, (unsigned short) 0x8AE5,
    (unsigned short) 0x2206, (unsigned short) 0x00E0,
    (unsigned short) 0x2206, (unsigned short) 0x8AFC,
    (unsigned short) 0x2206, (unsigned short) 0xE18A,
    (unsigned short) 0x2206, (unsigned short) 0xFDE2,
    (unsigned short) 0x2206, (unsigned short) 0x85C0,
    (unsigned short) 0x2206, (unsigned short) 0xE385,
    (unsigned short) 0x2206, (unsigned short) 0xC102,
    (unsigned short) 0x2206, (unsigned short) 0x2DAC,
    (unsigned short) 0x2206, (unsigned short) 0xAD20,
    (unsigned short) 0x2206, (unsigned short) 0x12EE,
    (unsigned short) 0x2206, (unsigned short) 0x8AE4,
    (unsigned short) 0x2206, (unsigned short) 0x03EE,
    (unsigned short) 0x2206, (unsigned short) 0x8AE5,
    (unsigned short) 0x2206, (unsigned short) 0xB7EE,
    (unsigned short) 0x2206, (unsigned short) 0x85C0,
    (unsigned short) 0x2206, (unsigned short) 0x00EE,
    (unsigned short) 0x2206, (unsigned short) 0x85C1,
    (unsigned short) 0x2206, (unsigned short) 0x00AE,
    (unsigned short) 0x2206, (unsigned short) 0x1115,
    (unsigned short) 0x2206, (unsigned short) 0xE685,
    (unsigned short) 0x2206, (unsigned short) 0xC0E7,
    (unsigned short) 0x2206, (unsigned short) 0x85C1,
    (unsigned short) 0x2206, (unsigned short) 0xAE08,
    (unsigned short) 0x2206, (unsigned short) 0xEE85,
    (unsigned short) 0x2206, (unsigned short) 0xC000,
    (unsigned short) 0x2206, (unsigned short) 0xEE85,
    (unsigned short) 0x2206, (unsigned short) 0xC100,
    (unsigned short) 0x2206, (unsigned short) 0xFDFC,
    (unsigned short) 0x2206, (unsigned short) 0x0400,
    (unsigned short) 0x2205, (unsigned short) 0xE142,
    (unsigned short) 0x2206, (unsigned short) 0x0701,
    (unsigned short) 0x2205, (unsigned short) 0xE140,
    (unsigned short) 0x2206, (unsigned short) 0x0405,
    (unsigned short) 0x220F, (unsigned short) 0x0000,
    (unsigned short) 0x221F, (unsigned short) 0x0000,
    (unsigned short) 0x133E, (unsigned short) 0x000E,
    (unsigned short) 0x133F, (unsigned short) 0x0010,
    (unsigned short) 0x13EB, (unsigned short) 0x11BB,
    (unsigned short) 0x207F, (unsigned short) 0x0002,
    (unsigned short) 0x2073, (unsigned short) 0x1D22,
    (unsigned short) 0x207F, (unsigned short) 0x0000,
    (unsigned short) 0x133F, (unsigned short) 0x0030,
    (unsigned short) 0x133E, (unsigned short) 0x000E,
    (unsigned short) 0x2200, (unsigned short) 0x1340,
    (unsigned short) 0x133E, (unsigned short) 0x000E,
    (unsigned short) 0x133F, (unsigned short) 0x0010,
    (unsigned short) 0x2000, (unsigned short) 0x1940,
    (unsigned short) 0x2060, (unsigned short) 0x1940,
    (unsigned short) 0x2080, (unsigned short) 0x1940,
    (unsigned short) 0x2017, (unsigned short) 0xA100,
    (unsigned short) 0x2077, (unsigned short) 0xA100,
    (unsigned short) 0x2097, (unsigned short) 0xA100,
    (unsigned short) 0x203F, (unsigned short) 0x0002,
    (unsigned short) 0x2031, (unsigned short) 0x7FF0,
    (unsigned short) 0x203F, (unsigned short) 0x0000,
    (unsigned short) 0x18E0, (unsigned short) 0x4004,
    (unsigned short) 0x1303, (unsigned short) 0x0778,
    (unsigned short) 0x1304, (unsigned short) 0x7777,
    (unsigned short) 0x13E2, (unsigned short) 0x01FE,
    (unsigned short) 0x1305, (unsigned short) 0xC001,
    (unsigned short) 0x1310, (unsigned short) 0x1076,
    (unsigned short) 0x1305, (unsigned short) 0xC011,
    (unsigned short) 0x1311, (unsigned short) 0x1076,
    (unsigned short) 0x1306, (unsigned short) 0x000F,
    (unsigned short) 0x1307, (unsigned short) 0x000F,
};

/**
 * @brief MDIO operation of switch chip for reading from internal register of RTL8363SB chip
 *
 * @param [in]  bus     Pointer to standard U-boot/Linux type MDIO bus data structure
 * @param [in]  addr    16 bit value of rtl8363SB internal register address of Switch to read value from
 *
 * @returns 16 bit value read (if unable to read, may return 0x0000 or 0xFFFF)
 */
unsigned short rtl8363sbReadRegister (struct mii_bus * bus, unsigned short addr)
{
    // Select address and read mode:
    bus->write(bus,0,0x1D,0xFFFF);
    bus->write(bus,0,0x1F,0x000E);
    bus->write(bus,0,0x1D,0xFFFF);
    bus->write(bus,0,0x17,  addr);
    bus->write(bus,0,0x1D,0xFFFF);
    bus->write(bus,0,0x15,0x0001);
    bus->write(bus,0,0x1D,0xFFFF);
    // Read data:
    return (unsigned short)(bus->read(bus,0,0x19));
}

/**
 * @brief MDIO operation of switch chip for writing to internal register of RTL8363SB chip
 *
 * @param [in]  bus     Pointer to standard U-boot/Linux type MDIO bus data structure
 * @param [in]  addr    16 bit value of RTL8363SB internal register address of Switch to read value from
 * @param [in]  value   16 bit value to write to RTL8363SB internal register
 *
 * @returns 16 bit value read (if unable to read, may return 0x0000 or 0xFFFF)
 */
void rtl8363sbWriteRegister (struct mii_bus * bus, unsigned short addr, unsigned short value)
{
    // Setup for Write, Write address and data value, Activate write to internal register:
    bus->write(bus,0,0x1D,0xFFFF);
    bus->write(bus,0,0x1F,0x000E);
    bus->write(bus,0,0x1D,0xFFFF);
    bus->write(bus,0,0x17,  addr);
    bus->write(bus,0,0x1D,0xFFFF);
    bus->write(bus,0,0x18, value);
    bus->write(bus,0,0x1D,0xFFFF);
    bus->write(bus,0,0x15,0x0003);
}

/**
 * @brief MDIO operation of RTL8363SB to read RJ45 PHY port register
 *
 * @param [in]  bus     Pointer to standard U-boot/Linux type MDIO bus data structure
 * @param [in]  phyID   8 bit ID number (0-1) of internal Switch PHY device to read data from
 * @param [in]  phyAddr 8 bit value of rtl8363SB internal register address of Switch to read value from
 *
 * @returns 16 bit value read (if unable to read, may return 0x0000 or 0xFFFF)
 */
unsigned short rtl8363sbReadPhyRegister (struct mii_bus * bus, unsigned short phyID, unsigned char phyAddr)
{
    return(rtl8363sbReadRegister(bus,(unsigned short)((0x2000+(phyID<<5)+phyAddr))));
}

/**
 * @brief MDIO operation of RTL8363SB to write RJ45 PHY port register
 *
 * @param [in]  bus     Pointer to standard U-boot/Linux type MDIO bus data structure
 * @param [in]  phyID   8  bit ID number (0-1) of internal Switch PHY device to read data from
 * @param [in]  phyAddr 8  bit value of rtl8363SB internal register address of Switch to read value from
 * @param [in]  value   16 bit value of data to write to the PHY register
 *
 * @returns 16 bit value read (if unable to read, may return 0x0000 or 0xFFFF)
 */
void rtl8363sbWritePhyRegister(struct mii_bus * bus, unsigned short phyID, unsigned char phyAddr, unsigned short value)
{
    rtl8363sbWriteRegister(bus,(unsigned short)((0x2000+(phyID<<5)+phyAddr)),value);
}

/**
 *
 * @brief rtl8363sb_phy_is_link_alive - test to see if the specified link is alive
 *
 * @return RETURNS:
 *    - TRUE  --> link is alive
 *    - FALSE --> link is down
 */
BOOL
rtl8363sb_phy_is_link_alive(struct mii_device *mdev,int phyUnit)
{

    //
    // Dummy function for now, always return true
    //
        return TRUE;
}

/**
 *
 * @brief Function to read and print the status of all PHY ports that are active
 *
 */

int
rtl8363sb_phy_stat(struct mii_device *mdev)
{
    uint16_t  phyHwStatus;
    uint32_t  phyAddr;
    int       phyUnit;
    int       ii = 200;

    //
    // DUMMY FOR NOW...
    //
    return 0;
}

/**
 *
 * @brief rtl8363sb_phy_init - reset and setup the PHY associated
 *
 * Resets the associated PHY port.
 *
 * @return RETURNS:
 *    - TRUE  --> associated PHY is alive
 *    - FALSE --> no LINKs on this ethernet unit
 */

int
rtl8363sb_phy_init(struct mii_device *mdev, int phyUnit)
{
    // PHY init currently part of baseline init
    // Currently PHY reinit is not implemented
    // This function is currently a placeholder
    // for possible future use

      return TRUE; // DUMMY FOR NOW, ALWAYS RETURN TRUE

}

int
rtl8363sb_phy_setup(struct mii_device *mdev, int phyUnit)
{
    int i;

    rtl8363sb_phy_init(mdev, phyUnit);
    return TRUE;
}


unsigned short rtl8363sb_phy_debug_read (struct mii_device * mdev,
                                         unsigned char        phy_ID,
                                         unsigned char        reg_addr
                                        )
{
    unsigned short reg_val;

    reg_val = rtl8363sbReadPhyRegister(
        mdev,     // struct mii_bus * bus,
        phy_ID,   // unsigned short   phyID,
        reg_addr  // unsigned char    phyAddr
        );

    INFO_PRINT("%s:  phy_ID 0x%4.4X reg 0x%4.4X val 0x%4.4X\n",__func__, phy_ID, reg_addr, reg_val);
    mdelay(10); // Delay for print to complete

    return (unsigned int) reg_val;
}

void rtl8363sb_phy_debug_write(
         struct mii_device * mdev,
         unsigned char       phy_ID,
         unsigned char       reg_addr,
         unsigned short      reg_val
         )
{
#ifdef DEBUG_REALTEK_SWITCH
    INFO_PRINT("%s: phy_addr 0x%4.4X reg 0x%4.4X val 0x%4.4X\n",__func__, phy_addr, reg_addr, (uint16_t)reg_val);
    mdelay(10); // Delay for print to complete
#endif
    rtl8363sbWritePhyRegister (
        mdev,     // struct mii_bus * bus,
        phy_ID,   // unsigned short   phyID,
        reg_addr, // unsigned char    phyAddr
        reg_val
        );

}

void DumpRealTekStatistics(struct mii_device *mdev, int MacPortNumber)
{
#if 0
    // PLACEHOLDER FOR NOW UNTIL IMPLEMENTED
    unsigned int i;
    unsigned int val;

    INFO_PRINT("RealTek MIB statistics port %u:",MacPortNumber);

    for (i=STATISTICS_START; i<=STATISTICS_END; i++)
    {
        if ((i-STATISTICS_START) % 0xF == 0)
        {
            INFO_PRINT("\nReg 0x%4.4X:",i);
        }
        val = rtl8363sbReadRegRegister(mdev,i);

        INFO_PRINT(" %8.8X",val);
    }
    INFO_PRINT("\n");
#endif
}

/** @brief Dump All Realtek Register */
void DumpRealTekRegisters(struct mii_device * mdev)
{
    unsigned int i,j,k,value,phy;

    INFO_PRINT("Dumping Realtek Switch PHY registers:\n");

    phy = 0;

    for (j=k=0; phy<2; j++,k++)
    {
        k &= 15;
        j &= 31;
        if (j==0)
        {
            INFO_PRINT("\nPHY %u:",phy);
        }
        if (k==0)
            INFO_PRINT("\n%04X:",j);
        value = rtl8363sbReadPhyRegister(mdev,phy,j);
        INFO_PRINT(" %04X", value);
        if (j==31)
            phy++;
    }
    INFO_PRINT("\n");

#if 0
    INFO_PRINT("Dumping Realtek Switch Internal registers:\n");

    for (i=j=k=0; i<0x2000; i++,j++,k++)
    {
        k &= 15;
        j &= 0xFF;
        if (j==0)
            INFO_PRINT("\n");
        if (k==0)
            INFO_PRINT("\n%04X:",i);
        value = rtl8363sbReadRegRegister(mdev,i);
        INFO_PRINT(" %04X", value);
    }
    INFO_PRINT("\n");
#endif

}

/**
    @brief Configure RealTek RTL8363SB switch for PC73300 T3300
           Dual-Mode radio reference design
*/
UINT32 CfgRealTekSwitch(struct mii_bus * bus)
{
    INFO_PRINT("%s: ",__func__);
    int i;
    unsigned short addr;
    unsigned short reg;
    unsigned short value;

    /*
     * Check if RealTek Switch present
     * Write 0x5555 to a "safe" internal register and read back
     *
     * This software may run on board without this chip, so if the read back value fails,
     * we assume the chip is not present and we skip initialization
     */
    INFO_PRINT("Checking for RTL8363SB chip: ");
    rtl8363sbWriteRegister(
        bus,    // struct mii_bus * bus,
        0x13A0, // unsigned short   addr,
        0x5555  // unsigned short   value
        );
    value = rtl8363sbReadRegister(
        bus,    // struct mii_bus * bus,
        0x13A0  // unsigned short   addr
        );
    if (value != 0x5555)
    {
        INFO_PRINT("Not detected, skipping initialization\n");
        return SYSRC_SUCCESS;
    }
    /*
     * Able to read and write back to internal register
     * Assume switch is fully operational and do a bulk
     * setting of all registers from pre-configured table
     *
     * Initialize RealTek 8363SB switch using pre-built data table
     * customized for PC73300 board where:
     *  - Both EXT ports are setup for RGMII, fixed gigabit speed,
     *    fixed full duplex, flow control enabled
     *  - Primary RJ45 port is PHY port 0
     *  - Secondary debug RJ45 PHY port is port 1
     *  - Both RJ45 PHY ports are setup for 10/100/1000 MBPS, Full/Half Duplex,
     *    full autonegotiation
     */
    INFO_PRINT("Initializing, ");
    for (i=0; i<(sizeof(rtl8362sbInitTable)/sizeof(rtl8362sbInitTable[0])); i+=2)
    {
        addr  = rtl8362sbInitTable[i];
        value = rtl8362sbInitTable[(i+1)];
        rtl8363sbWriteRegister(
            bus,  // struct mii_bus * bus,
            addr, // unsigned short   addr,
            value // unsigned short   value
            );
    }
    INFO_PRINT("Done\n");

#if 0
    DumpRealTekRegisters();
#endif

    return SYSRC_SUCCESS;

}

UINT32 SetRealTekSwitchLoopback(struct mii_bus *bus, int enabled)
{
    if (enabled)
    {
        // PLACEHOLDER FOR REALTEK SPECIFIC CODE
    }
    else
    {
        // PLACEHOLDER FOR REALTEK SPECIFIC CODE
    }
    return SYSRC_SUCCESS;
}

#ifdef LINUX
EXPORT_SYMBOL(rtl8363sb_init)
EXPORT_SYMBOL(rtl8363sb_phy_setup)
EXPORT_SYMBOL(rtl8363sb_phy_stat)
EXPORT_SYMBOL(rtl8363sb_reg_read)
EXPORT_SYMBOL(rtl8363sb_reg_write)
EXPORT_SYMBOL(rtl8363sb_phy_debug_read)
EXPORT_SYMBOL(rtl8363sb_phy_debug_write)
#endif
