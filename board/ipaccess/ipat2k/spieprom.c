/**
 * @file  spieeprom.c
 *
 * @brief SPI EEPROM driver and diagnostics
 *
 * @copyright
 * Copyright(c) 2014-2015 Intel Corporation. All rights reserved.
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
 ******************************************************************************/
#include "asm/arch/spieprom.h"
#define SYSRC_AT25DRV_CHIP_BOUNDARY         0x00000801


typedef struct gSpiEepromDataStruct
{
    UINT32 CommandLength;
    UINT32 DataLength;
    UINT8  Data[8];
} SPI_EEPROM_COMMAND_DATA_TYPE, * P_SPI_EEPROM_COMMAND_DATA_TYPE;

SPI_EEPROM_COMMAND_DATA_TYPE gSpiEepromCommandData[2];

SPI_EEPROM_DEVICE gSpiEpromData[2];

/** @brief SPI driver transfer (read and write) descriptors, 2, one for each SPI bus */
S_SPI_TRANSFER_DES gSpiEpromMessageDescriptor[2];

/**
 * @brief Function to get Number of address bytes to send when sending
 * SPI commands to the SPI EEPROM
 *
 */
UINT32 Eprom_GetNumberOfAddressBytes(PFLASH_DEVICE pFlash)
{
    if (pFlash->TotalSize <= 65536)
    {
        // Size of EEPROM is 64 kilobybtes or less, return
        // number of bytes to use in address fields of SPI commands as 2
        // (16 bits, 2 x 8 bit fields, Most Significant byte first)
        //
        return 2;
    }
    else
    {
        //
        // Size of EEPROM is greater than 64 kilobytes, return
        // 3 bytes as required (24 bit addressing)
        //
        return 3;
    }

}

/**
 * @brief Function to initialize SPI HW for operation with SPI EEPROM
 * @details        Initialize SPI controller for operation with SPI EEPROM
 *                 Device.  For example clock phase control
 *                 clock polarity select,master or slave mode selcet and data
 *                 direction select (MSB first or LSB first).
 *
 */
UINT32 Eprom_HWInit(P_SPI_EEPROM_DEVICE pDevice)
{
    UINT32 rc         = SYSRC_SUCCESS;
    SPICONFIG SpiConfig;
    UINT32 busID      = 1;
    /*If chipselect is greater than 3 then it is GPIO controlled 
      We are using 18 for LS SPI CS 0 is mapped on GPIO 18*/ 
    UINT32 chipSelect = EEPROM_CS_GPIO;

    //
    // Setup SPI configuration for controller operation
    // with standard SPI EEPROM
    //
    memset(&SpiConfig, 0, sizeof(SpiConfig));

    SpiConfig.frameSize     = SPI_FRAME_SIZE_8BITS; // 8 bits per transfer
    SpiConfig.frameFormat   = SPI_MOTOROLA_FORMAT;  // Motorola SPI format
    SpiConfig.clkMode       = SPI_CLOCK_MODE3;      // Idle clock high, Data valid on second clock edge
    SpiConfig.transferMode  = SPI_RX_TX_ENABLE;     // Full duplex enabled
    SpiConfig.frqDivider    = SPI_DIVIDER_SPEED_MHZ(10); // TEMP fixed speed @ 10 MHz for both busses

    SpiConfig.BusID         = 1;                // Bus ID as requested by parameter

    rc = SPIDrvInit(&SpiConfig);

    if (chipSelect > 3)
    {
        //
        // GPIO control, setup SPI chip select for default state
        // GPIO as GPIO, output and set to HIGH for inactive
        // state for chip select
        //
        EEPROM_CS_AS_GPIO(chipSelect);
        EEPROM_CS_AS_OUTPUT(chipSelect);
        //
        // Microchip 25AA512 spec says CS must be low prior to
        // any operation.
        // Set low for a while then set back to
        // high to wake up the chip
        //
        // This also should not hurt other chips
        //
        EEPROM_CS_LOW(chipSelect);
        __udelay(2000000);
        EEPROM_CS_HIGH(chipSelect);
    }

    return rc;
}

/**
 * @brief Function to send a simple one byte command with no data to read
 */
void Eprom_SendOneByteCommand(P_SPI_EEPROM_DEVICE pDevice, UINT8 Command)
{
    UINT32 busID      = pDevice->BusID;
    UINT8  chipSelect = pDevice->ChipSelect;
    UINT8  command    = Command;

    gSpiEpromMessageDescriptor[busID].command_ptr  = &command;
    gSpiEpromMessageDescriptor[busID].cmd_size     = 1;
    gSpiEpromMessageDescriptor[busID].data_size    = 0;
    gSpiEpromMessageDescriptor[busID].tx_data_ptr  = NULL;
    gSpiEpromMessageDescriptor[busID].rx_data_ptr  = NULL;

    SPIDrvTransfer(busID, chipSelect, &gSpiEpromMessageDescriptor[busID]);
    return;
}

/**
 * @brief Function to send a simple one byte command with one byte of data to read
 */
UINT8 Eprom_SendOneByteCommandOneByteResponse(P_SPI_EEPROM_DEVICE pDevice, UINT8 Command)
{
    UINT32 busID      = pDevice->BusID;
    UINT8  chipSelect = pDevice->ChipSelect;
    UINT8  command    = Command;
    UINT8  response   = 0xFF;

    gSpiEpromMessageDescriptor[busID].command_ptr  = &command;
    gSpiEpromMessageDescriptor[busID].cmd_size     = 1;
    gSpiEpromMessageDescriptor[busID].data_size    = 1;
    gSpiEpromMessageDescriptor[busID].tx_data_ptr  = NULL;
    gSpiEpromMessageDescriptor[busID].rx_data_ptr  = &response;

    SPIDrvTransfer(busID, chipSelect, &gSpiEpromMessageDescriptor[busID]);
    return response;
}

/**
 * @brief Function to send a command already built in the command strcture with optional pointers
 * to Data to get
 */
void Eprom_SendCommand(P_SPI_EEPROM_DEVICE pDevice, UINT8 * pTxData, UINT8 * pRxData, UINT32 DataLength)
{
    UINT32 busID      = pDevice->BusID;
    UINT8  chipSelect = pDevice->ChipSelect;
    P_SPI_EEPROM_COMMAND_DATA_TYPE pEepromCommandData;

    //
    // Get pointer for global structure data for this bus
    //
    pEepromCommandData = &gSpiEepromCommandData[busID];

    gSpiEpromMessageDescriptor[busID].command_ptr  = pEepromCommandData->Data;
    gSpiEpromMessageDescriptor[busID].cmd_size     = pEepromCommandData->CommandLength;
    gSpiEpromMessageDescriptor[busID].data_size    = DataLength;
    gSpiEpromMessageDescriptor[busID].tx_data_ptr  = pTxData;
    gSpiEpromMessageDescriptor[busID].rx_data_ptr  = pRxData;

    SPIDrvTransfer(busID, chipSelect, &gSpiEpromMessageDescriptor[busID]);
    return;
}

/**
 * @brief Function to setup send SPI command to SPI EEPROM
 *
 */
void Eprom_SetupCommand(P_SPI_EEPROM_DEVICE pDevice, UINT8 Command)
{
    UINT32                         busID      = pDevice->BusID;
    P_SPI_EEPROM_COMMAND_DATA_TYPE pEepromCommandData;

    //
    // Get pointer for global structure data for this bus
    //
    pEepromCommandData = &gSpiEepromCommandData[busID];

    //
    // Clear command data structure
    //
    memset(pEepromCommandData,0,sizeof(*pEepromCommandData));

    //
    // Setup command
    //
    pEepromCommandData->Data[0]       = Command;
    pEepromCommandData->CommandLength = 1;
}

void Eprom_AddCommandByte(P_SPI_EEPROM_DEVICE pDevice, UINT8 CommandData)
{
    UINT32                         busID      = pDevice->BusID;
    P_SPI_EEPROM_COMMAND_DATA_TYPE pEepromCommandData;
    UINT32                         i;

    //
    // Get pointer for global structure data for this bus
    //
    pEepromCommandData = &gSpiEepromCommandData[busID];
    //
    // Put byte and increment length
    //
    i = pEepromCommandData->CommandLength;
    pEepromCommandData->Data[i] = CommandData;
    i++;
    pEepromCommandData->CommandLength = i;
}


/**
 * @brief Function to send 2 or 3 byte address cycle part of command to SPI EEPROM
 *
 */
void Eprom_SetupCommandAndAddress(P_SPI_EEPROM_DEVICE pDevice, UINT8 Command, UINT32 Address)
{
    UINT8  Addr0,Addr1,Addr2;
    UINT32 NumberOfAddressBytes = pDevice->NumAddrBytes;

    Eprom_SetupCommand(pDevice, Command);

    Addr0 = (UINT8) Address;
    Addr1 = (UINT8)(Address >> 8);
    Addr2 = (UINT8)(Address >> 16);

    //send address MSB first
    if (NumberOfAddressBytes != 2)
    {
        Eprom_AddCommandByte(pDevice, Addr2);
    }
    Eprom_AddCommandByte(pDevice, Addr1);
    Eprom_AddCommandByte(pDevice, Addr0);
}

/**
 * @brief Function to turn on SPI EEPROM write enable
 * @details        this instruction must been executed before Page Progream
 *                ,Sector Erase,Bulk Erase and Write Status register instruction.
 *
 */
void Eprom_WriteEnable(P_SPI_EEPROM_DEVICE pDevice)
{
    Eprom_SendOneByteCommand(pDevice, cmd_WR_ENABLE);
    Eprom_Wait       (pDevice);
}
/**
 * @brief Function to turn off (disable) writes to EEPROM
 *
 * @details        this instruction use to disable Page Progream
 *                ,Sector Erase,Bulk Erase and Write Status register instruction.
 *                Note that the WEL bit is automaticlly reset after power down and
 *                 upon completion of the Write Status Register, Page Program,Sector
 *                Erase and Buld Erase Instuction.
 *
 */
void Eprom_WriteDisable(P_SPI_EEPROM_DEVICE pDevice)
{
    Eprom_SendOneByteCommand(pDevice, cmd_WR_DISABLE);
    Eprom_Wait       (pDevice);
}

/**
 * @brief Function to write to SPI EEPROM status register
 */
void Eprom_WriteStatus(P_SPI_EEPROM_DEVICE pDevice, UINT8 Data)
{
    Eprom_WriteEnable(pDevice);
    Eprom_Wait       (pDevice);

    Eprom_SetupCommand(pDevice,cmd_WD_STATUS);
    Eprom_AddCommandByte(pDevice,Data);
    Eprom_SendCommand(pDevice,NULL,NULL,0);
    Eprom_Wait       (pDevice);
}

/**
 * @brief Function to read status register from SPI EEPROM
 * @details        this function use to read  the decvice status
 *                register. the read status register instruction maybe
 *                perform any time
 *
 */
void Eprom_ReadStatus(P_SPI_EEPROM_DEVICE pDevice, UINT8 * status)
{

    *status = Eprom_SendOneByteCommandOneByteResponse(pDevice, cmd_RD_STATUS);
}

/**
 * @brief        this function is used to wait until device internal operaton complete.
 */
void Eprom_Wait(P_SPI_EEPROM_DEVICE pDevice)
{
    UINT8 status;
    do
    {
        Eprom_ReadStatus(pDevice, &status);
    }
    while(status & BUSY);

}
/**
 * @brief    this function allows one or more data bytes to be sequentially read from the memory of device.
 */
void Eprom_ReadData(P_SPI_EEPROM_DEVICE pDevice, UINT8 *buf, UINT32 StartAddr, UINT32 Length)
{

    Eprom_Wait(pDevice);

    Eprom_SetupCommandAndAddress(pDevice, cmd_RD_DATA, StartAddr);

    Eprom_SendCommand(pDevice,NULL,buf,Length);
}
/**
 * @brief Function to program a page of memory with data from 1 to 256 bytes
 * @details        this function allows from one byte to 256 bytes of data
 *                to be programmed at memory locations previously erased
 *                to all 0xff.
 *
 */
void Eprom_PageProgram(P_SPI_EEPROM_DEVICE pDevice, UINT8 *buf, UINT32 StartAddr, UINT16 Length)
{
    Eprom_WriteEnable(pDevice);

    Eprom_Wait(pDevice);

    Eprom_SetupCommandAndAddress(pDevice, cmd_PAGE_PROGRAM, StartAddr);

    Eprom_SendCommand(pDevice,buf,NULL,Length);

    Eprom_Wait(pDevice);
   
}

/**
 * @brief Function to erase a single sector of the SPI EEPROM
 * @details        this function sets all memory within a specified sector
 *                to the erased state of all 0xff. a write enable instruction
 *                must be executed before the device will accept the erase
 *                sector instruction.the lowest 16 address bits must set to 0
 *
 */
UINT32 Eprom_SectorErase(P_SPI_EEPROM_DEVICE pDevice, UINT8 SectorNum)
{
    UINT32 rc         = SYSRC_SUCCESS;
    UINT32 StartAddr;

    Eprom_WriteEnable(pDevice);
    Eprom_Wait(pDevice);

    StartAddr = SectorNum * pDevice->SectorSize;

    Eprom_SetupCommandAndAddress(pDevice, cmd_SECTOR_ERASE, StartAddr);
    Eprom_SendCommand(pDevice,NULL,NULL,0);
    Eprom_Wait(pDevice);

    return rc;
}

/**
 * @brief Function to issue a command to the SPI EEPROM to erase the entire chip
 */
UINT32 Eprom_ChipErase(P_SPI_EEPROM_DEVICE pDevice)
{
    UINT32 rc         = SYSRC_SUCCESS;

    Eprom_WriteEnable(pDevice);
    Eprom_Wait(pDevice);

    Eprom_SendOneByteCommand(pDevice, cmd_CHIP_ERASE);
    Eprom_Wait(pDevice);

    return rc;
}

/**
 * @brief Function to put the SPI EEPROM device into a power down state
 * @details        while in the power-down state only the release from
 *                Power-down/Device ID instruction, which restores the
 *                device to normal operation, will be recognized
 *
 */
void Eprom_PowerDown(P_SPI_EEPROM_DEVICE pDevice)
{
    Eprom_SendOneByteCommand(pDevice, cmd_POWER_DOWN);
    Eprom_Wait(pDevice);
}
/**
 * @brief Function to take the SPI EEPROM out of Power down state
 */
void Eprom_ReleasePowerDown(P_SPI_EEPROM_DEVICE pDevice)
{
    UINT16 i;

    Eprom_SendOneByteCommand(pDevice, cmd_RELEASE_POWER_DOWN);

    for (i=0; i < 1000; i++);            //delay for select signal holding high

    Eprom_Wait(pDevice);
}

//#define EEPROM_SCOPE_READ_LOOP

/**
 * @brief Debug function to continually read status from
 * SPI EEPROM, used for board debugging using oscilloscope
 */
void EepromScopeReadLoop(P_SPI_EEPROM_DEVICE pDevice)
{
#ifdef EEPROM_SCOPE_READ_LOOP
    UINT32 RunTest = 1;
    UINT8  Status;
    UINT32 busID   = pDevice->BusID;

    while (RunTest)
    {
        //
        // Send read status command
        //
        Status = Eprom_SendOneByteCommandOneByteResponse(pDevice, cmd_RD_STATUS);
        //
        // Delay for 2 milliseconds
        //
        __udelay(2000000);
        //
        // Loop until stoppped by debugger
        //
    }
#endif
}

/**
 * @brief        Function to read device id and manufacture id from the SPI EEPROM device
 */
void Eprom_ReadID(P_SPI_EEPROM_DEVICE pDevice, AT25ID * IdInfo)
{

    UINT32 busID          = pDevice->BusID;
    UINT8  chipSelect     = pDevice->ChipSelect;
    UINT8  command;
    UINT8  ManufacturerID = 0;


    //
    // New code using descriptor for bus 0
    //
    if (busID == 1)
    {
        //
        // TEMP, ONLY READ ID FOR NON-MICROCHIP DEVICE for bus 0
        //
        command = cmd_ID;
        gSpiEpromMessageDescriptor[busID].command_ptr  = &command;
        gSpiEpromMessageDescriptor[busID].cmd_size     = 1;
        gSpiEpromMessageDescriptor[busID].data_size    = 3;
        gSpiEpromMessageDescriptor[busID].tx_data_ptr  = NULL;
        gSpiEpromMessageDescriptor[busID].rx_data_ptr  = (UINT8 *)IdInfo;

        SPIDrvTransfer(busID, chipSelect, &gSpiEpromMessageDescriptor[busID]);
        return;

    }

    //
    // Autodetect if ST or Microchip EEPROM
    // Send a release power down which will also get the
    // Manufacturer ID (at least this has been tested with Microchip EEPROM
    //
    // Microchip spec says to send a release power down command followed
    // by 16 bit address (2 x 8 bit writes) followed by a read 8 bit value
    // to get manufacturer ID
    //
    Eprom_SetupCommand(pDevice, cmd_RELEASE_POWER_DOWN);
    Eprom_AddCommandByte(pDevice, 0);
    Eprom_AddCommandByte(pDevice, 0);
    //
    // Send command as setup and read back one byte of data into
    // ManufacturerID
    //
    Eprom_SendCommand(pDevice,NULL,&ManufacturerID,1);

#ifdef EEPROM_SCOPE_READ_LOOP
    //
    // Test for checking SPI bus signal on oscilloscope by doing
    // continuous status read
    //
    EepromScopeReadLoop(pDevice);
#endif

    if (ManufacturerID == MICROCHIP_MANUFACTURER_ID)
    {
        IdInfo->ManufacturerID = ManufacturerID; // Store manufacturer ID
        //
        // Microchip 25AA512 does not have a command to get a device ID,
        // so fill device ID with 0x25AA for debug purposes
        //
        IdInfo->DeviceID1      = 0x25;
        IdInfo->DeviceID2      = 0xAA;
    }
    else
    {
        //
        // Not Microchip, assume ST EEPROM device
        //
        // Send read ID command
        //
        Eprom_SetupCommand(pDevice, cmd_ID);
        //
        // Send command and setup receive 3 bytes of data to id info field
        //
        Eprom_SendCommand(pDevice,NULL,(UINT8 *)&IdInfo,3);
        //
        // Whatever now comes back (good or bad) return it
        //
    }
}

/**
 * @brief Function to globally write protect the SPI EEPROM device
 */
void Eprom_GlobalProtect(P_SPI_EEPROM_DEVICE pDevice)
{
    UINT8 status;

    Eprom_ReadStatus(pDevice, &status);

    Eprom_WriteStatus(pDevice, status|PROTECT_BITS);
}
/**
 * @brief Function to globally write enable (all sectors) for the SPI EEPROM device
 */
void Eprom_GlobalUnProtect(P_SPI_EEPROM_DEVICE pDevice)
{
    UINT8 status;

    Eprom_ReadStatus(pDevice, &status);

    Eprom_WriteStatus(pDevice, status & (~PROTECT_BITS));
}

/**
 * @brief Function to erase a selected set of sectors based on address (base) and block count (number of sectors)
 */
static UINT32 SpiEprom_erase(PFLASH_DEVICE pFlash, UINT32 addr, UINT32 block_cnt)
{
    UINT32              block;
    UINT32              rc;
    P_SPI_EEPROM_DEVICE pDevice = (P_SPI_EEPROM_DEVICE)pFlash;

    if (addr % (pFlash->BlockSize))
    {
        printf("SPI Eprom: Failed erase, block boundary");
        return SYSRC_AT25DRV_CHIP_BOUNDARY;
    }

    if (block_cnt == 0)
        return SYSRC_SUCCESS;

    if(addr + block_cnt * pFlash->BlockSize > pFlash->TotalSize)
    {
        printf("SPI Eprom: Failed erase, chip boundary");
        return SYSRC_AT25DRV_CHIP_BOUNDARY;
    }

    block = addr / (pFlash->BlockSize);
    for (;block_cnt > 0;)
    {
        rc = Eprom_SectorErase(pDevice,
                               block
                              );
        block_cnt--;
        block++;

        if (rc != SYSRC_SUCCESS)
            return rc;
    }

    return SYSRC_SUCCESS;
}

/**
 * @brief Function to write a selected set of sectors based on address (base) and block count (number of sectors) from RAM memory
 */
static UINT32 SpiEprom_write(PFLASH_DEVICE pFlash, UINT8* pOutBuf, UINT32 addr, UINT32 len)
{
    P_SPI_EEPROM_DEVICE pDevice = (P_SPI_EEPROM_DEVICE)pFlash;
    UINT32 pageSize,column;


    if (!len)
        return SYSRC_SUCCESS;

    if (addr + len > pFlash->TotalSize)
    {
        printf("SPI Eprom: Failed write, chip boundray");
        return SYSRC_AT25DRV_CHIP_BOUNDARY;
    }

    pageSize = pDevice->PageSize;
    column   = addr % pageSize;

    while (1)
    {
        if(column)
        {
            Eprom_PageProgram(pDevice, pOutBuf, addr, pageSize - column);
            len     -= (pageSize - column);
            pOutBuf += (pageSize - column);
            addr    += (pageSize - column);
        }
        else if (len < pageSize)
        {
            if (len > 0)
            {
                Eprom_PageProgram(pDevice, pOutBuf, addr, len);
            }
            break;
        }
        else
        {
            Eprom_PageProgram(pDevice, pOutBuf, addr, pageSize);
            len     -= pageSize;
            pOutBuf += pageSize;
            addr    += pageSize;
        }
        column = 0;
    }

    return SYSRC_SUCCESS;
}

/**
 * @brief Function to read a selected set of sectors based on address (base) and block count (number of sectors) into RAM memory
 */
static UINT32 SpiEprom_read(PFLASH_DEVICE pFlash, UINT8* pInBuf, UINT32 addr, UINT32 len)
{
    P_SPI_EEPROM_DEVICE pDevice = (P_SPI_EEPROM_DEVICE)pFlash;

    if (!len)
        return SYSRC_SUCCESS;

    if (addr + len > pFlash->TotalSize)
    {
        printf("SPI Eprom: Failed read, chip boundray");
        return SYSRC_AT25DRV_CHIP_BOUNDARY;
    }

    Eprom_ReadData(pDevice, pInBuf,addr,len);

    return SYSRC_SUCCESS;
}

/**
 * @brief Function to initialize SPI EEPROM driver, detect device and setup for subsequent operations
 */
UINT32 SpiEprom_init(PFLASH_DEVICE pFlash)
{
    P_SPI_EEPROM_DEVICE pDevice = (P_SPI_EEPROM_DEVICE)pFlash;
    AT25ID IdInfo;
    UINT32 busID;

    memset((void *)&gSpiEpromMessageDescriptor[0],0,sizeof(gSpiEpromMessageDescriptor[0]));
    memset((void *)&gSpiEpromMessageDescriptor[1],0,sizeof(gSpiEpromMessageDescriptor[1]));

    memset((void *)&IdInfo,0,sizeof(IdInfo));

    Eprom_HWInit(pDevice);

    if (pDevice->IOBase == (PVOID)SPI0_BASEADDR)
    {
        busID = 0;
        //
        // Setup variables for T2200 and T3300 board
        // that has high speed SPI device on high speed
        // SPI bus 0
        //
        pDevice->ChipSelect = HS_EEPROM_CS_SPI; // TEMP FORCE TO ZERO UNTIL ACTUAL CS SETUP
    }
    else
    {
        pDevice->IOBase = (PVOID)SPI1_BASEADDR;  // Force correc base address if not high speed SPI
        //
        // Setup Chip select for SPI EEPROM device on Mindspeed
        // Transcede boards for low speed SPI bus 1
        //
        //pDevice->ChipSelect = EEPROM_CS_SPI;
        pDevice->ChipSelect = EEPROM_CS_GPIO;
        busID = 1;
    }

    pDevice->BusID = busID;

    /* Read ID info from EEPROM */
    Eprom_ReadID(pDevice, &IdInfo);

    /* Safety check: Make sure not returned a NULL pointer (should not happen) */


    /* Store manufacturer and device ID info */
    pFlash->MfgID    =  IdInfo.ManufacturerID;
    pFlash->DeviceID = (IdInfo.DeviceID1 << 8) | IdInfo.DeviceID2;

    //
    // Test if Supported device
    //
    if (pFlash->MfgID == MICROCHIP_MANUFACTURER_ID)
    {
        //
        // Microchip device
        // Adjust Device Data fields for Microchip instead of AT25DF161
        //
        pDevice->TotalSector  = MICROCHIP_25AA512_TOTAL_SECTOR;
        pDevice->SectorSize   = MICROCHIP_25AA512_SECTOR_SIZE;
        pDevice->PageSize     = MICROCHIP_25AA512_PAGE_SIZE;
    }
    //
    // Not microchip, check for ST or ATMEL (same parameters)
    //
    else 
    {
        // Setup AT25DF161/AT25DQ161  device parameters
        pDevice->TotalSector  = AT25DF161_TOTAL_SECTOR;
        pDevice->SectorSize   = AT25DF161_SECTOR_SIZE;
        pDevice->PageSize     = AT25DF161_PAGE_SIZE;
    }
    //
    // Unknown device, return error
    //

    /* Setup Flash Device interface*/
    pFlash->baseAddress = 0;  // Flash base address is serial, so use 0 offset for generic flash driver // was (UINT32) pDevice->IOBase;
    pFlash->BlockSize   = pDevice->SectorSize;
    pFlash->TotalSize   = pDevice->TotalSector * pDevice->SectorSize;

    /* Setup number of address bytes */
    pDevice->NumAddrBytes = Eprom_GetNumberOfAddressBytes(pFlash);

    /* Setup Flash Device interface calls for generic read, write and erase functions */
    pFlash->read  = SpiEprom_read;   /* Setup call for read  function */
    pFlash->write = SpiEprom_write;  /* Setup call for write function */
    pFlash->erase = SpiEprom_erase;  /* Setup call for erase function */

    /* Unprotect the EEPROM */
    Eprom_GlobalUnProtect(pDevice);

    return SYSRC_SUCCESS;
}

/*function to erase and reprogram the SPI chip*/

void program_spi_loader(UINT32 mem_addr,UINT32 len)
{
    UINT32 ret;
    SPI_EEPROM_DEVICE   SPIEEPROMDev;
    UINT32     rc = SYSRC_SUCCESS;
    memset(&SPIEEPROMDev, 0x00, sizeof(SPIEEPROMDev));
    SPIEEPROMDev.ChipSelect = 0;
    SPIEEPROMDev.dev.baseAddress = SPI1_BASEADDR;
    SPIEEPROMDev.IOBase          = (PVOID) SPI1_BASEADDR;
    SPIEEPROMDev.BusID           = 1;
    
    /*Initialize the SPI Chip attached on LS SPI 0 */
    rc = SpiEprom_init((PFLASH_DEVICE)&SPIEEPROMDev);

    if (rc == SYSRC_SUCCESS)
    {
            printf("SPI EEPROM init OK, Mfg. ID: 0x%02X Device ID: 0x%04X OK\n",
                   SPIEEPROMDev.dev.MfgID,
                   SPIEEPROMDev.dev.DeviceID
                   );
	    printf("Erasing the chip first...\n");
	    rc = Eprom_ChipErase((PFLASH_DEVICE)&SPIEEPROMDev);
	    if(rc == SYSRC_SUCCESS)
	    {
		    printf("Writing to SPI\n");
		    rc = SpiEprom_write((PFLASH_DEVICE)&SPIEEPROMDev, (UINT8 *)mem_addr, 0, len);
		    if(rc == SYSRC_SUCCESS)
		    {
			    printf("Writing to SPI Successful\n");
		    }
                    else
                    {
			    printf("SPI EEPROM Write failed, error 0x%08X\n",rc);
                    }
	    }
            else
            {
                   printf("SPI EEPROM Erase failed, error 0x%08X\n",rc);

            }
    }
    else
    {
            printf("SPI EEPROM device unable to initialize, error 0x%08X Mfg. ID: 0x%02X, Device ID: 0x%04X\n",
                   rc,
                   SPIEEPROMDev.dev.MfgID,
                   SPIEEPROMDev.dev.DeviceID
                  );
    }
}


