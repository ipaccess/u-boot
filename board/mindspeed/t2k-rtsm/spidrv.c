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

#include <config.h>
#include <common.h>
#include <asm/arch/spidrv.h>


const P_SPI_REGISTER_STRUCT BusIdToSpiControllerPointer[2] =
{
    (P_SPI_REGISTER_STRUCT)SPI0_BASEADDR,
    (P_SPI_REGISTER_STRUCT)SPI1_BASEADDR
};

#define GET_SPI_CONTROLLER_POINTER(BUS_ID) BusIdToSpiControllerPointer[(BUS_ID & 1)]

//
// TODO: Need to eventually add DMA support for SPI 0 high speed
// bus as an initializationi option.
// For now, both busses are running in sending max of 8 x 16 bit
// words to egress FIFO (and receiving same or less amount at
// the same time)
//
UINT32 SPIDrvInit(PSPICONFIG pSpiConfig)
{
    volatile UINT16 dummy;
    volatile P_SPI_REGISTER_STRUCT pSpiController;
    UINT32 busID;

    busID = pSpiConfig->BusID;

    pSpiController = GET_SPI_CONTROLLER_POINTER(busID);

    //
    // T2200 for SPI interface, change default of SPI slave select from debug
    // mode to enable Slave Select 3 for SPI bus 0
    //
    REG32(MISC_PIN_SELECT_REG) |= 1;

    if (busID != 0)
    {
        //
        // SPI bus 1
        //
        // Make sure SPI TX and RX for SPI bus 1 (low speed bus) is not setup as GPI
        // This should be defaulted for this, but set this in case
        // some other software change the pins to GPIO
        //
        REG32(GPIO_31_16_PIN_SELECT_REG) &= ~((0x0fUL << 28) | (0x0fUL << 4)); // TODO: here we enable both SS0 and SS1, define only one really used
        //REG32(GPIO_31_16_PIN_SELECT_REG) &= ~(0x0fUL << 28); // Clear bits 31-28 to enable TX and RX controlled by SPI controller instead of GPIO
        //
        // Set maximum drive strength for clock and TX data and slave selects and fast I/O
        //
        REG32(PAD_CONF2_REG) = 0x33333;
    }
    else
{
        //
        // SPI bus 0
        //
        // Set maximum drive strength for clock and TX data
        //
        REG32(PAD_CONF1_REG) = 0x33;
    }


#if 0
    //Set up spi chip select GPIO pin configuration
    // Clear bits 31-28 and 7-4 to setup SPI_S1 TXD, RXD, SS0 and SS1 controlled by SPI controller
    // NOTE: removed to allow defaults to run and calling function using SPI driver
    // to control whether it wants to use GPIO or not
    REG32(GPIO_31_16_PIN_SELECT_REG &= ~(0x0f << 4); // TODO: here we enable both SS0 and SS1, define only one really used
#endif



    pSpiController->SSIENR = 0x0;            // disable SPI

    // b0 - b3         : transfer size
    // b4 - b5         : Synchronous Serial protocol type (usually Motorola SPI)
    // b6              : Clock polarity
    // b7              : state of serial
    // b8 - b9         : Transmit & receive control (duplex control)
    // b10             : TX enable
    //*(UINT32 *)SPI_CTRLR0 = 0x00c7; // 8-bit interface
    pSpiController->CTRLR0 = (pSpiConfig->frameSize    << 0) | /* 3:0 Data frame size */
                             (pSpiConfig->frameFormat  << 4) | /* 5:4 Frame format 00:SPI, 01:SSP, 10:Microwire*/
                             (pSpiConfig->clkMode      << 6) | /* 7:6 (Valid when SPI): 6:Phase, 7:Polarity */
                             (pSpiConfig->transferMode << 8);  /* 9:8 Transfer mode 00:TX&RX, 01:TX, 10:RX */

    pSpiController->BAUDR  = pSpiConfig->frqDivider; // run it at speed as specified by caller
    pSpiController->IMR    = 0;                      // disable all interrupts
    pSpiController->RXFTLR = 0;                      // interrupt is asserted when 1 or more valid data entry is present
    pSpiController->SER    = 0;                      // disable individual slave chip select pin
    pSpiController->SSIENR = 0x1;                    // enable SPI

    //
    // Clear receive FIFO of any leftover/extraneous receive SPI data
    //
    while (pSpiController->RXFLR != 0)
    {
        dummy = pSpiController->DR;
    }

    return SYSRC_SUCCESS;
}

/* Function to write a single 16 or 8 bits of data */
UINT16 SPIDrvWrite(UINT32 busID, UINT32 cs, UINT16 data)
{
    volatile P_SPI_REGISTER_STRUCT pSpiController;

    pSpiController = GET_SPI_CONTROLLER_POINTER(busID);

    SPIDrvEnableCS(busID, cs);

    // Send data to SPI
    pSpiController->DR = data;

    // Wait for completion
    while (pSpiController->RXFLR == 0);

    // Get data read from SPI bus (i.e. data from other direction at the same time as write occurred)
    data = pSpiController->DR;

    SPIDrvDisableCS(busID, cs);

	return data;
}

UINT16 SPIDrvRead(UINT32 busID, UINT32 cs)
{
    UINT16 value;
    volatile P_SPI_REGISTER_STRUCT pSpiController;

    pSpiController = GET_SPI_CONTROLLER_POINTER(busID);


    SPIDrvEnableCS(busID, cs);

    // Send dummy data (0xFF) to SPI
    pSpiController->DR = 0xFF;

    // Wait for completion
    while (pSpiController->RXFLR == 0);

    // Get data read from SPI bus (i.e. data from other direction at the same time as dummy write occurred)
    value = pSpiController->DR;

    SPIDrvDisableCS(busID, cs);

	return value;
}

#ifdef USE_GPIO_CTRL_CS
void SPISendByte(UINT32 busID, UINT8 Data)
{
    volatile UINT16 value;
    P_SPI_REGISTER_STRUCT pSpiController;

    // Get pointer to SPI controller registers based on bus ID
    pSpiController = GET_SPI_CONTROLLER_POINTER(busID);

    // Send 8 bit data to SPI controller data register to start transmission
    pSpiController->DR = Data;

    // Sending 8 bits of data for TX will also start receiving data in RX FIFO
    // Wait until there is one byte of data in the RX FIFO indicating
    // TX has completed and there is one byte of dummy data on
    // the RX data which needs to be read and discarded

    while (pSpiController->RXFLR == 0);

    // Read dummy RX (i.e. don't care) data
    value = pSpiController->DR;

}

UINT8 SPISendReadByte(UINT32 busID, UINT8 Data)
{
    volatile UINT16 value;
    P_SPI_REGISTER_STRUCT pSpiController;

    // Get pointer to SPI controller registers based on bus ID
    pSpiController = GET_SPI_CONTROLLER_POINTER(busID);

    //
    // Send 8 bit data to SPI controller data register to start transmission
    // NOTE: For case of read only byte transaction, this data may
    // be dummy data (0x00 or 0xFF)
    //
    pSpiController->DR = Data;

    //
    // Sending 8 bits of data for TX will also start receiving data in RX FIFO
    // Wait until there is one byte of data in the RX FIFO indicating
    // TX has completed and there is one byte of data on
    // the RX data which needs to be read and returned
    // as data to the calling software
    //
    while (pSpiController->RXFLR == 0);

    //
    // Get receive data from Data register
    //
    value = pSpiController->DR;

    return ((UINT8)value);
}
#endif

void SPIDrvEnableCS(UINT32 busID, UINT32 cs)
{
    volatile P_SPI_REGISTER_STRUCT pSpiController;

    pSpiController = GET_SPI_CONTROLLER_POINTER(busID);

    pSpiController->SSIENR = 0x0;
    pSpiController->SER   |= cs;
    pSpiController->SSIENR = 0x1;
}

void SPIDrvDisableCS(UINT32 busID, UINT32 cs)
{
    volatile P_SPI_REGISTER_STRUCT pSpiController;

    pSpiController = GET_SPI_CONTROLLER_POINTER(busID);

    pSpiController->SSIENR = 0x0;
    pSpiController->SER   &= ~cs;
    pSpiController->SSIENR = 0x1;
}

void SPIDrvSetClockMode(UINT32 busID, UINT32 mode)
{
    volatile P_SPI_REGISTER_STRUCT pSpiController;

    pSpiController = GET_SPI_CONTROLLER_POINTER(busID);

    pSpiController->SSIENR = 0x0; // disable SPI
    pSpiController->CTRLR0 = (pSpiController->CTRLR0 & ~(3 << 6)) | mode; // Set clocking mode
    pSpiController->SSIENR = 0x1; // enable SPI
}

/**
 * @brief Function to write a basic SPI read or write operation common to many SPI based chips
 * To write a 16 bit address and do an 8 bit read
 *
 * @param busID           [in] - SPI bus ID, 0 for high speed SPI bus, 1 for low speed SPI bus
 * @param chipSelect      [in] - Chip select on this bus, 0-3 for SPI controller based, otherwise, uses GPIO
 * @param RegisterAddress [in] - 16 bit register address to send on write command
 * @param Data            [in] - 8 bit data to send after register
 *
 * @return - Returns 8 bit read from SPI that occurs at the same time that 3rd byte (real or dummy) is written
 */
UINT32 SpiDrvReadWriteReg(UINT32 busID, UINT32 chipSelect, UINT16 RegisterAddress, UINT8 Data)
{
    UINT16 cw = RegisterAddress;
    UINT32 val;
    UINT32 spi_cs; // SPI chip select for controller, if GPIO, then controller cs 0 is used
    UINT32 gpio_val;
    UINT8  byte1;
    UINT8  byte2;

    volatile P_SPI_REGISTER_STRUCT pSpiController;

    pSpiController = GET_SPI_CONTROLLER_POINTER(busID);

    if (chipSelect <= 3)
    {
        spi_cs   = 1 << chipSelect;
        gpio_val = 0;
    }
    else
    {
        if ((chipSelect == 19) && (busID==1))
            spi_cs   = (1<<1); // GPIO 19 mapped to slave select 1 on SPI bus 1
        else
            spi_cs   = (1<<0); // GPIO 18 on SPI bus 1 or other GPIO, use slave select 0

        gpio_val = 1 << chipSelect;
    }

    SPIDrvEnableCS(busID, spi_cs);

    //
    // GPIO value, set active low
    //
    if (gpio_val)
    {
        REG32(GPIO_OUTPUT_REG) &= ~(gpio_val);
    }
    //
    // Prep data so we can send it fast to the FIFO (so it doesn't underrun)
    //
    byte1 = cw >> 8;
    byte2 = cw & 0xFF;

    //
    // Send 16 bit address and one real or dummy byte to the SPI controller FIFO
    //
    pSpiController->DR = byte1;
    pSpiController->DR = byte2;
    pSpiController->DR = Data;
    //
    // Wait for 3 x 8 bit data to be received into FIFO
    //
    while (pSpiController->RXFLR < 3);
    //
    // Discard first 2 x 8 bit data values
    //
    val = pSpiController->DR;
    val = pSpiController->DR;
    //
    // Get desired 8 bit data to read from bus
    //
    val = pSpiController->DR;
    //
    // Disable GPIO (if using GPIO for actual chip select)
    //
    if (gpio_val)
{
        REG32(GPIO_OUTPUT_REG) |= gpio_val;
    }
    //
    // Disable slave select for SPI controller (real slave select line, or zero if GPIO used)
    //
    SPIDrvDisableCS(busID, spi_cs);
#if 0
    //
    // For debug, Add a little delay between each read or write command
    // Also this makes it easier on an oscilloscope to see individual transactions
    //
    SysDelay(2);
#endif
#ifdef GENERATE_XML
    //
    // TEMP FOR CREATING XML FOR TOTALPHASE AARDVARK TOOL, GENERATE XML OUTPUT
    //
    printf("  <spi_write count=\"3\" radix=\"16\">%02X %02X %02X</spi_write>\n",
           byte1,
           byte2,
           Data
          );
    printf("  <sleep ms=\"10\" />\n");
#endif
#ifdef DEBUG_SPI
    //
    // TEMP FOR NOW, DEBUG ALL TRANSACTIONS TO COMPARE WITH SPI ANALYZER
    //
    printf("\n MOSI: %02X %02X %02X MISO: %02X ",
           byte1,
           byte2,
           Data,
           val
          );
#endif
    //
    // Return Last 8 bit value read
    //
    return val;
}

/**
 * @brief Function to write a basic SPI write operation common to many SPI based chips
 * To write a 16 bit address and do an 8 bit read where the most significant bit of the 16
 * bit address is 1 for doing a write operation and 0 for doing a read
 *
 * @param busID           [in] - SPI bus ID, 0 for high speed SPI bus, 1 for low speed SPI bus
 * @param chipSelect      [in] - Chip select on this bus, 0-3 for SPI controller based, otherwise, uses GPIO
 * @param RegisterAddress [in] - 16 bit register address to send on write command (highest bit set to indicate write operation to device)
 * @param Data            [in] - 8 bit data to send after register
 *
 * @return -  void return
 *
 */
void SpiDrvWriteReg(UINT32 busID, UINT32 chipSelect, UINT16 RegisterAddress, UINT8 Data)
{

    (void)SpiDrvReadWriteReg(busID,
                             chipSelect,
                             RegisterAddress | 0x8000,
                             Data
                            );
}

/**
 * @brief Function to write a basic SPI read or write operation common to many SPI based chips
 * To write a 16 bit address and do an 8 bit read
 *
 * @param busID           [in] - SPI bus ID, 0 for high speed SPI bus, 1 for low speed SPI bus
 * @param chipSelect      [in] - Chip select on this bus, 0-3 for SPI controller based, otherwise, uses GPIO
 * @param RegisterAddress [in] - 16 bit register address to send on read command
 *
 * @return - Returns 8 bit read from SPI that occurs at the same time that 3rd byte (dummy byte) is written
 */
UINT8 SpiDrvReadReg(UINT32 busID, UINT32 chipSelect, UINT16 RegisterAddress)
{
    UINT8 SpiDataReadFromBus;

    SpiDataReadFromBus =
        SpiDrvReadWriteReg(busID,
                           chipSelect,
                           RegisterAddress,
                           0
                          );
    return (SpiDataReadFromBus);
}

/** @brief Function to do a data transfer of a single command with full duplex data
 *
 * @details
 *
 * @par NOTE
 * NOTE: This function will only support a maximum size of what the TX
 * FIFO can send.
 *
 * For the lower speed SPI controller Bus 1, this is 8 entries.
 *
 * For the high speed SPI controller Bus 0, this is ??? entries
 *
 */
UINT32 SPIDrvTransfer(UINT32 busID, UINT32 cs, S_SPI_TRANSFER_DES * ptrans_des)
{
    UINT32          i;
    UINT32          j;
    UINT32          TxCmdCount;
    UINT32          DataSize = ptrans_des->data_size;
    volatile UINT16 dummy;
    UINT32          startTime;
    UINT32          timeout   = EXPBUSTICKS_IN_1_MS; // Timeout, 1 millisecond
    UINT32          SlaveSelect;

    volatile P_SPI_REGISTER_STRUCT pSpiController;

    /* Get Pointer to SPI controller register structure based on BUS ID passed */
    pSpiController = GET_SPI_CONTROLLER_POINTER(busID);

    //
    // Setup Chip Select.
    // If zero through 3, use SPI controller Slave Select 0-3 and no GPIO
    // If greater than 3, use GPIO and use spi controller slave select 0
    //
    if (cs > 3)
    {
        //
        // GPIO, set chip select GPIO to low and use
        // Slave select 0
        //
        SlaveSelect = 1;
        CS_LOW(cs);
    }
    else
    {
        //
        // Not GPIO, setup slave select 0 thorugh 3 based on parameter
        // passed
        //
        SlaveSelect = 1 << cs;
    }

    //if((ptrans_des->data_size >0) && ((ptrans_des->tx_data_ptr == NULL) || (ptrans_des->rx_data_ptr == NULL)))
    //    return 1;


    //
    // Now go ahead and enable SPI controller chip select
    // to start the transfer
    //
    SPIDrvEnableCS(busID, SlaveSelect);
    //
    // Send command info first to allow some data to go into the FIFO
    // prior to enabling transfer by enabling chip select in the controller
    // This helps avoid FIFO underruns and allows for higher speed SPI
    // rates to be used.
    //
    // Send command cycle (note must fit in FIFO
    for(i= 0, j=ptrans_des->cmd_size; i < j; i++)
    {
        pSpiController->DR = ptrans_des->command_ptr[i];
}


    //
    // Copy comamnd data sent count to variable to be used in
    // loop below
    //
    TxCmdCount = j;

    //
    // Loop through data (if any) and send it to the SPI controller as long as
    // there is room available in the egress FIFO
    //
    for(i=j=0; ((i<DataSize) || (j < DataSize) || (TxCmdCount > 0)); )
    {
        //
        // First test if anything to send (or are we done)
        //
        if (i < DataSize)
        {
            //
            // If room in FIFO send any data (real or dummy) to TX
            //
            //if (pSpiController->TXFLR < (SPI_FIFO_SIZE-1))
            if (pSpiController->SR & SPI_SR_TFNF) // Check status register for Transmit FIFO not full
            {
                //
                // There is still more data to send and room in FIFO
                // Test if there is real data to
                // send or if not, send dummy data
                //
                if (ptrans_des->tx_data_ptr != NULL)
                {
                    //
                    // There is real data, send one byte
                    // of real data
                    //
                    pSpiController->DR = ptrans_des->tx_data_ptr[i];
                }
                else
                {
                    //
                    // There is no real data to send, send dummy byte
                    //
                    pSpiController->DR = 0xFF;
                }
                i++; // One byte of data sent (real or dummy)
            }
        }
        //
        // TX processed for this loop iteration,
        // Do any necessary RX Processing
        //
        // See if there is any data in the RX FIFO to process
        //
        //if (pSpiController->RXFLR > 0)
        if (pSpiController->SR & SPI_SR_RFNE) // Check status register for receive FIFO not empty
        {
            //
            // There is data to process
            // First check if read occuring during the
            // command cycle need to be flushed first
            // (those reads are always discarded by this function)
            //
            if (TxCmdCount > 0)
            {
                //
                // RX data occurred during command cycle, discard it
                //
                dummy = pSpiController->DR;
                TxCmdCount--;
            }
            else
            {
                //
                // Command cycle is complete,
                // Check if any Data to receive
                //
                if ((ptrans_des->rx_data_ptr != NULL) && (j < DataSize))
                {
                    //
                    // There is real data to receive and count
                    // not exceeded, get byte from RX FIFO and
                    // store to memory
                    //
                    ptrans_des->rx_data_ptr[j] = pSpiController->DR ;
                }
                else
                {
                    //
                    // There is no real data to recieve,
                    // Pull dummy data out of the RX FIFO
                    // and discard it.
                    //
                    dummy = pSpiController->DR;
                }
                j++; // One byte of data read (real or dummy)
            }
        }
    }

    //
    // All bytes (real and dummy ones) sent and received
    // For safety, make sure RX and TX FIFOs are fully empty
    //
    startTime = SysTime;
    while ((pSpiController->RXFLR > 0) && (pSpiController->TXFLR > 0))
    {
        if (pSpiController->RXFLR > 0)
{
            dummy = pSpiController->DR;
}

        if(SysTime - startTime > timeout)
{
            return 1;
}
    }

    // All bytes transmitted and received, disable slave select for this transaction

    SPIDrvDisableCS(busID, SlaveSelect);

    //
    // Put GPIO back to high if GPIO line
    //
    if (cs > 3)
    {
        CS_HIGH(cs);
    }

    return SYSRC_SUCCESS;
}
