/**
 * @file  spieeprom.h
 * @brief Defines, structures and exported function calls for SPI EEPROM driver and diagnostics
 *
 * @copyright
 * Copyright(c) 2014 Intel Corporation. All rights reserved.
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
 *
 */
#ifndef __EPROM_H__
#define __EPROM_H__

#include "asm/arch/flashtypes.h"
#include "asm/arch/spidrv.h"


/***************************************************************************
**  The error codes of the module, it's used by module interface functions *
****************************************************************************/
#define SPI_EEPROM_DRV                      0x17
#define RC_AT25DRV_OK                       MX_OK
#define RC_AT25DRV_ID_ERROR                 DEF_USRRC(APP_DRIVER,SPI_EEPROM_DRV, 1)
#define RC_AT25DRV_PROGRAM_LEN_ERROR        DEF_USRRC(APP_DRIVER,SPI_EEPROM_DRV, 2)
#define RC_AT25DRV_CMD_LEN_ERROR            DEF_USRRC(APP_DRIVER,SPI_EEPROM_DRV, 3)
#define RC_AT25DRV_BLOCK_BOUNDARY           DEF_USRRC(APP_DRIVER,SPI_EEPROM_DRV, 4)
#define RC_AT25DRV_CHIP_BOUNDARY            DEF_USRRC(APP_DRIVER,SPI_EEPROM_DRV, 5)
#define RC_AT25DRV_SECTOR_NUM_ERROR         DEF_USRRC(APP_DRIVER,SPI_EEPROM_DRV, 6)

/* Defines for AT25DF161 512 Kilobytes SPI EEPROM device
 * NOTE: AT25DQ161 has same values
 */
#define AT25DF161_PAGE_SIZE             256
#define AT25DF161_TOTAL_SIZE            8*512*1024    /* Total size (4 Mbits)512 Kilobytes*/
#define AT25DF161_SECTOR_SIZE           64*1024       /*66 KiloBytes Sector Size)*/
#define AT25DF161_TOTAL_SECTOR          8 

/* Defines for Microchip 25AA512 512 kilobit SPI EEPROM device */
#define MICROCHIP_25AA512_PAGE_SIZE     128            // Page size in bytes
#define MICROCHIP_25AA512_TOTAL_SIZE    64*1024        // Total size 512 kilobits (64 kilobytes)
#define MICROCHIP_25AA512_SECTOR_SIZE   16*1024
#define MICROCHIP_25AA512_TOTAL_SECTOR  4



/*************** command define******************/
//Read Commmans
#define cmd_RD_DATA                 0X03
#define cmd_FAST_RD                 0X0B

//Program and Erase Commands
#define cmd_SECTOR_ERASE            0XD8    //BLOCK 64K for AT25xxx device, 16K for 25AA512 device
#define cmd_CHIP_ERASE              0XC7
#define cmd_PAGE_PROGRAM            0X02

//Protection Commands
#define cmd_WR_ENABLE               0X06    // write enable command
#define cmd_WR_DISABLE              0X04    // write disable command

//Status Commands
#define cmd_RD_STATUS               0X05    // read status register command
#define cmd_WD_STATUS               0X01    // write status register command

//Miscellaneous Commands
#define cmd_ID                      0X9F    // Get ID (note not supported by Microchip device)
#define cmd_POWER_DOWN              0XB9
#define cmd_RELEASE_POWER_DOWN      0XAB


/**************** DEVICE IDS ***********************/
#define ATMEL_MANUFACTURER_ID       0X1F
#define ATMEL_DENSITY_MASK          0X1F /* Density ID is in least significant 5 bits of 1st Device ID byte */
#define ATMEL_DENSITY_16_MBIT       0X06

#define ST25P_MANUFACTURER_ID       0X20
#define ST25P16_TYPE_ID             0X20
#define ST25P16_CAPACITY_ID         0X15
#define ST25P16_ID                  ((ST25P16_TYPE_ID<<8) | ST25P16_CAPACITY_ID)

#define MICROCHIP_MANUFACTURER_ID   0X29

#define DUMMY_DATA                  0XFF
/************************** PIN define **************/

#ifndef USE_GPIO_CTRL_CS
/** @brief Define to use direct GPIO on chip select for SPI rather than using SPI controller to control chip select */
#define USE_GPIO_CTRL_CS
#endif

/** @brief On T2200 ASIC checkout board, low speed SPI EEPROM on SPI bus 1, SS0 */
#define EEPROM_CS_GPIO              CS_SPI_1_SS0_N_GPIO

/** @brief SPI controller programming of chip select for low speed EEPROM */
#define EEPROM_CS_SPI               0

/** @brief Low speed SPI EEPROM is on low speed SPI bus 1 for Mindspeed boards */
#define EEPROM_BUS_ID               1

/** @brief SPI controller programming of chip select for high speed EEPROM
 *
 * NOTE: High speed eeprom is on SPI 0 bus.  The SPI 0 bus does not have the
 * to map any SPI slave selects to GPIOs.
 *
 * For standard T2200 EVMs, standard T3300 EVM this is mapped to slave select 1 on bus ID 0
 *
 * NOTE: For T2200 ASIC checkout board, it is assumed the user has jumpered the slave select
 * to SS1 as it can be jumpered for any of the four slave selects.
 */
#define HS_EEPROM_CS_SPI            1

/** @brief High speed SPI EEPROM is on low speed SPI bus 0 for Mindspeed boards */
#define HS_EEPROM_BUS_ID            0

#define EEPROM_CS_HIGH(GPIO)         CS_HIGH(GPIO)
#define EEPROM_CS_LOW(GPIO)          CS_LOW(GPIO)
#define EEPROM_CS_AS_GPIO(GPIO)      CS_AS_GPIO(GPIO)
#define EEPROM_CS_AS_OUTPUT(GPIO)    CS_AS_OUTPUT(GPIO)


/**** status register bit define ******/
#define SRWD                        0X80
#define BP2                         0X10
#define BP1                         0X08
#define BP0                         0X04
#define WEL                         0X02
#define BUSY                        0X01

#define PROTECT_BITS                (BP2|BP1|BP0)

/**
 * @brief SPI EEPROM AT25 class device ID information structure
 *
 * @par NOTE
 * NOTE: Also used for other SPI EEPROM devices as well, not just AT25 class devices
 *
 */
typedef struct tagAT25ID
{
    UINT8    ManufacturerID;
    UINT8    DeviceID1;
    UINT8    DeviceID2;

}AT25ID;

/** @brief SPI based Flash device control block structure */
typedef struct
{
    FLASH_DEVICE  dev; /**< @brief Data for generic common Flash device structure */

    PVOID  IOBase;      /**< @brief Pointer to SPI memory controller register base */
    UINT32 TotalSector; /**< @brief Total number of sectors in this Flash device */
    UINT32 SectorSize;  /**< @brief Number of bytes per sector in this Flash device */
    UINT32 PageSize;    /**< @brief Number of bytes per page   in this Flash device */
    UINT32 nCfg;
    UINT32 BusID;       /**< @brief SPI bus number that EEPROM is attached to */
    UINT32 ChipSelect;  /**< @brief SPI bus chip select (0-3 for SPI controller control, else GPIO #) */
    UINT32 NumAddrBytes;/**< @brief Number of address bytes for this device */

} SPI_EEPROM_DEVICE,*P_SPI_EEPROM_DEVICE;


/************function declare***********/
UINT32  Eprom_HWInit          (P_SPI_EEPROM_DEVICE pDevice);
void    Eprom_WriteStatus     (P_SPI_EEPROM_DEVICE pDevice, UINT8 Data);
void    Eprom_ReadStatus      (P_SPI_EEPROM_DEVICE pDevice, UINT8 * status);
void    Eprom_Wait            (P_SPI_EEPROM_DEVICE pDevice);
void    Eprom_ReadData        (P_SPI_EEPROM_DEVICE pDevice, UINT8 *buf, UINT32 StartAddr, UINT32 Length);
void    Eprom_PageProgram     (P_SPI_EEPROM_DEVICE pDevice, UINT8 *buf, UINT32 StartAddr, UINT16 Length);
UINT32  Eprom_SectorErase     (P_SPI_EEPROM_DEVICE pDevice, UINT8 sectorNum);
UINT32  Eprom_ChipErase       (P_SPI_EEPROM_DEVICE pDevice);
void    Eprom_PowerDown       (P_SPI_EEPROM_DEVICE pDevice);
void    Eprom_ReleasePowerDown(P_SPI_EEPROM_DEVICE pDevice);
void    Eprom_ReadID          (P_SPI_EEPROM_DEVICE pDevice, AT25ID * IdInfo);
void    Eprom_WriteEnable     (P_SPI_EEPROM_DEVICE pDevice);
void    Eprom_WriteDisable    (P_SPI_EEPROM_DEVICE pDevice);
void    Eprom_GlobalProtect   (P_SPI_EEPROM_DEVICE pDevice);
void    Eprom_GlobalUnProtect (P_SPI_EEPROM_DEVICE pDevice);

UINT32  SpiEprom_init         (PFLASH_DEVICE  pFlash);
void program_spi_loader(UINT32 mem_addr,UINT32 len);

//UINT32 SpiEprom_write(PFLASH_DEVICE pFlash, UINT8* pOutBuf, UINT32 addr, UINT32 len);

#endif
