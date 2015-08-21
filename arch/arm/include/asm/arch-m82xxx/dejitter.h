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

#ifndef __DEJITTER_H__
#define __DEJITTER_H__

#include "spidrv.h"

#define USE_GPIO_CS

#ifndef UINT32
#define UINT32 unsigned int
#endif

#ifndef VUINT32
#define VUINT32 volatile unsigned int
#endif

#ifndef PUINT32
#define PUINT32 unsigned int *
#endif

#ifndef UINT16
#define UINT16 unsigned short
#endif

#ifndef VUINT16
#define VUINT16 volatile unsigned short
#endif

#ifndef PUINT16
#define PUINT16 unsigned short *
#endif

#ifndef UINT8
#define UINT8 unsigned char
#endif

#ifndef PUINT8
#define PUINT8 unsigned char *
#endif

#ifndef VUINT8
#define VUINT8 volatile unsigned char
#endif

/** @brief Dejitter/PLL device main loop rate
 *
 * NOTE: This is programmable, so this constant must match however the chip is programmed
 * so proper divisors can be calculated
 */
#define DEJITTER_MAIN_LOOP_RATE_HZ 240000000

// Dejitter init options
#define DEJITTER_INIT_STARTUP       0   // Set IDLE and also burn to EEPROM if necessary
#define DEJITTER_INIT_IDLE          1
#define DEJITTER_INIT_CPRI          2
#define DEJITTER_INIT_RADIO         3
#define DEJITTER_INIT_PC73300       4  // MSPD PC73300 board, Cypress PLL only, 25 MHz input, 125 MHz output to GEM TSU
#define DEJITTER_INIT_ADF4002       5  // MSPD PC73300 board, Slave only, ADF4002 PLL only, 10 MHz input to discipline 25 MHz main oscillator
#define DEJITTER_INIT_MAX_VALUE     DEJITTER_INIT_ADF4002

// Clock sources (currently only supported on AD9548)
#define AD9548_CLK_SRC_REFA         0
#define AD9548_CLK_SRC_REFAA        1
#define AD9548_CLK_SRC_REFB         2
#define AD9548_CLK_SRC_REFBB        3
#define AD9548_CLK_SRC_REFC         4
#define AD9548_CLK_SRC_REFCC        5
#define AD9548_CLK_SRC_REFD         6
#define AD9548_CLK_SRC_REFDD        7
#define AD9548_CLK_SRC_MAX_VALUE    AD9548_CLK_SRC_REFDD

#define AD9548_CLK_OUT_MODE_CMOS_BOTH_PINS     (0<<0)
#define AD9548_CLK_OUT_MODE_CMOS_POSITIVE_PIN  (1<<0)
#define AD9548_CLK_OUT_MODE_CMOS_NEGATIVE_PIN  (2<<0)
#define AD9548_CLK_OUT_MODE_TRISTATE_BOTH_PINS (3<<0) /* Default @ reset */
#define AD9548_CLK_OUT_MODE_LVDS               (4<<0)
#define AD9548_CLK_OUT_MODE_LVPECL             (5<<0)

#define AD9548_CLK_OUT_CMOS_DRIVE_STRENGTH_LOW      (0<<3) /* Default @ reset */
#define AD9548_CLK_OUT_CMOS_DRIVE_STRENGTH_NORMAL   (1<<3)

#define AD9548_CLK_OUT_POLARITY_NORMAL              (0<<4) /* Default @ reset */
#define AD9548_CLK_OUT_POLARITY_INVERTED            (1<<4)

#define AD9548_CLK_OUT_CMOS_PINS_RELATIVE_PHASE_SAME     (0<<5) /* Default @ reset */
#define AD9548_CLK_OUT_CMOS_PINS_RELATIVE_PHASE_INVERTED (1<<5)


// T3300 and T2200 EVM mapping from clock sources to PLL inputs
#define DEJITTER_CLK_SRC_TRANSCEDE      AD9548_CLK_SRC_REFA  // Clock from Transcede Network timing Generator
#define DEJITTER_CLK_SRC_GPS_1PPS       AD9548_CLK_SRC_REFB  // GPS 1 PULSE PER SECOND (1 Hz)
#define DEJITTER_CLK_SRC_GPS_OSCILLATOR AD9548_CLK_SRC_REFBB // GPS MODULE DISCPLINED TIMING
#define DEJITTER_CLK_SRC_SMA_CONNECTOR  AD9548_CLK_SRC_REFD  // Clock from external source via coax cable to SMA
#define DEJITTER_CLK_SRC_RADIO          AD9548_CLK_SRC_REFDD // Clock out from Radio chipset

#define DEJITTER_CLK_SRC_FREE_RUN       (AD9548_CLK_SRC_MAX_VALUE+1)
#define DEJITTER_CLK_SRC_HOLDOVER       (AD9548_CLK_SRC_MAX_VALUE+2)
#define DEJITTER_CLK_SRC_AUTOMATIC      (AD9548_CLK_SRC_MAX_VALUE+3)

#define DEJITTER_CLK_SRC_MAX_PARAMETER  DEJITTER_CLK_SRC_AUTOMATIC

//
// Mapping of SPI chip selects and SPI bus (0 or 1) to Dejitter/Timing PLLs
//
/** @brief Mapping of AD9548 to SPI chip select
 *
 * On all EVMs, the AD9548 is mapped/connected to SPI chip select (slave select) 0
 *
 * NOTE: Revision 1 T2200 ASIC checkout boards originally set AD9548 chip select
 * to 1, this was changed to 0 via rework to make common with T2200 EVM and T3300 EVM.
 */
#define AD9548_SPI_CS       0

/** @brief Mapping of AD9548 to SPI bus ID
 *
 * On all EVMs, the AD9548 is mapped/connected to SPI bus ID 0
 */
#define AD9548_SPI_BUS_ID   0

/** @brief Mapping of AD9553 to SPI chip select
 *
 * On T2200 EVM, the AD9553 is mapped/connected to SPI chip select (slave select) 1
 * On other current EVMs this device is not present.
 */
#define AD9553_SPI_CS       1

/** @brief Mapping of AD9553 to SPI bus ID
 *
 * On T2200 EVM, the AD9553 is mapped/connected to SPI bus ID 0.
 * On other current EVMs this device is not present.
 */
#define AD9553_SPI_BUS_ID   0

/** @brief Mapping of ADF4002 to SPI chip select
 *
 * On PC73300 board , the ADF4002 is mapped/connected to SPI chip select (slave select) 1,
 * currently on the slave processor of the T3300 Transcede (NOTE: This may change to
 * T3300 master in the future)
 *
 * On other current EVMs this device is not present.
 */
#define ADF4002_SPI_CS       1

/** @brief Mapping of ADF4002 to SPI Bus ID
 *
 * On PC73300 board , the ADF4002 is mapped/connected to SPI bus 1,
 * currently on the slave processor of the T3300 Transcede (NOTE: This may change to
 * T3300 master in the future)
 *
 * On other current EVMs this device is not present.
 */
#define ADF4002_SPI_BUS_ID   1

// Function prototypes
void   DejitterInit  (UINT32 InitOption, UINT32 ClockSource);         // Initialize dejitter circuitry
void   DejitterSelectSource(UINT32 ClockSourceId);     // Setup Dejitter refernce clock input (if supported by circuitry)
void   DejitterSetFreeRunLoopHz(UINT32 LoopRateInHz);  // Force Free Run internal frequency to different value for testing
void   DejitterSetupOutput  (UINT8  ClockOutId, UINT8  ClockOutMode, UINT32 ClockOutSpeed); // Setup Dejitter output port for mode (chip specific) and speed (in Hz)

UINT32 DejitterRead  (UINT32 addr, UINT32 busID, UINT32 cs);                // Read  RAM value
void   DejitterWrite (UINT32 addr, UINT32 data, UINT32 busID, UINT32 cs);   // Write RAM value

void CypressPLLInit (UINT32 InitOption);

void SetI2cDacValue(unsigned short DacValue);
#endif
