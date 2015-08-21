#ifndef __SPIDRV_H__
#define __SPIDRV_H__

#include <asm/arch/hardware.h>

// Mapping of diagnostic types to standard C

#ifndef UINT32
#define UINT32 unsigned int
#endif

#ifndef VUINT32
#define VUINT32 volatile unsigned int
#endif

#ifndef UINT16
#define UINT16 unsigned short
#endif

#ifndef VUINT16
#define VUINT16 volatile unsigned short
#endif

#ifndef UINT8
#define UINT8 unsigned char
#endif

#ifndef SYSRC_SUCCESS
#define SYSRC_SUCCESS 0
#endif

unsigned int get_bus_clock(void);

#define EXPBUSTICKS_IN_1_US  (get_bus_clock())
#define EXPBUSTICKS_IN_1_MS ((get_bus_clock()) * 1000)

ulong get_timer (ulong base);
#define SysTime (get_timer(0))

/*
 * SPI addresses
 * NOTE: T2200 and T3300 have dual SPI busses, one high speed with
 * DMA (SPI bus 0) and one for lower speed (SPI bus 1)
 * Most devices and software from previous Transcede devices
 * only had to deal with a single SPI bus (same as SPI bus 1)
 *
 * For T2200 and T3300 software, two addresses are defined so that
 * whenever the SPI bus is used, the driver and the application
 * must know/specify which controller/bus to use
 */
#ifndef SPI0_BASEADDR
#define SPI0_BASEADDR                   0xFE804000
#endif

#ifndef SPI1_BASEADDR
#define SPI1_BASEADDR                   0xFE098000
#endif

/****************************************************
*													*
*  The list of devices' CS connected to SPI busses	*
*													*
****************************************************/

#ifndef USE_GPIO_CTRL_CS
/** @brief Define to use direct GPIO on chip select for SPI rather than using SPI controller to control chip select */
#define USE_GPIO_CTRL_CS
#endif

#ifdef USE_GPIO_CTRL_CS
//
// GPIO used to control SPI chip select
//

//
// Chip selects as GPIO for T2200 SPI busses (there are two SPI busses on the T2200)
//
#define CS_SPI_1_SS1_N_GPIO     19
#define CS_SPI_1_SS0_N_GPIO     18

//
// Macros for initializing and setting chip select high (inactive) or low (active/selected)
//

/** @brief Macro to set chip select to HIGH state (chip not selected) */
#define CS_HIGH(GPIO)		REG32(GPIO_OUTPUT_REG) |= (1<<GPIO)

/** @brief Macro to set chip select to Low state (chip selected) */
#define CS_LOW(GPIO)		REG32(GPIO_OUTPUT_REG) &= ~(1<<GPIO)

/** @brief Macro to setup SPI chipselect as GPIO (instead of mapped to controller for control) */
#define CS_AS_GPIO(GPIO)    REG32(GPIO_31_16_PIN_SELECT_REG) |= (1 << ((GPIO-16)*2))

/** @brief Macro to setup SPI chipselect as GPIO (instead of mapped to controller for control) */
#define CS_AS_OUTPUT(GPIO)  REG32(GPIO_OE_REG) |= (1 << GPIO)

#else
//
// SPI chip select controlled by SPI controller, not GPIO
//
#define CS_HIGH(GPIO)
#define CS_LOW(GPIO)
#define CS_AS_GPIO(GPIO)
#define CS_AS_OUTPUT(GPIO)
#endif

#define FPGA_BASE				0x10000000
#define FPGA_END				(FPGA_BASE + 0x8000000)

#define FPGA_RADIO0_CS_OFFSET	0x0
#define RADIO0_CS_DAC0_EN		(~(0x1<<0))
#define RADIO0_CS_ADC0_EN		(~(0x1<<1))
#define RADIO0_CS_DAC1_EN		(~(0x1<<2))
#define RADIO0_CS_ADC1_EN		(~(0x1<<3))

#define FPGA_RADIO1_CS_OFFSET	0x1000000
#define RADIO1_CS_DAC2_EN		(~(0x1<<0))
#define RADIO1_CS_ADC2_EN		(~(0x1<<1))
#define RADIO1_CS_DAC3_EN		(~(0x1<<2))
#define RADIO1_CS_ADC3_EN		(~(0x1<<3))

#define SPI_FRAME_SIZE_8BITS	0x07
#define SPI_FRAME_SIZE_16BITS	0x0F

#define SPI_MOTOROLA_FORMAT 	0x00

/*
 * Definition of Phase and Polarity for SPI clock modes:
 *
 * Phase 0: Serial Clcok toggles in the middle of the first data bit
 * Phase 1: Serial Clcok toggles at the start  of the first data bit
 *
 * Polarity 0: Incative state of serial clock is low
 * Polarity 1: Inactive state of serial clock is high
 */
#define SPI_CLOCK_MODE0             0x00  /**< @brief SPI clock mode 0 Bit 6:Phase=0, Bit 7:Polarity=0 */
#define SPI_CLOCK_MODE1             0x01  /**< @brief SPI clock mode 1 Bit 6:Phase=1, Bit 7:Polarity=0 */
#define SPI_CLOCK_MODE2             0x02  /**< @brief SPI clock mode 1 Bit 6:Phase=0, Bit 7:Polarity=1 */
#define SPI_CLOCK_MODE3             0x03  /**< @brief SPI clock mode 1 Bit 6:Phase=1, Bit 7:Polarity=1 */

#define SPI_RX_TX_ENABLE		0x00

#define SPI_DIVIDER_SPEED_1M        EXPBUSTICKS_IN_1_US  // Expansion bus speed from config.h

#define SPI_DIVIDER_SPEED_KHZ(x)    ((UINT32)((SPI_DIVIDER_SPEED_1M * 1000) / (UINT32)x) )
#define SPI_DIVIDER_SPEED_MHZ(x)    ((UINT32) (SPI_DIVIDER_SPEED_1M         / (UINT32)x) )

#define SPI_FIFO_SIZE			8		//Only 8 words of 16 bits width
#define SPI_COMMAND_MAX_SIZE   	8

//
// Misc. SPI controller define bits
//

//
// SPI controller status Register (SR) (Register offset 0x28)
//
#define SPI_SR_DCOL     (1<<6)  /**< @brief Transmit Data collision error (only used for multi-master) */
#define SPI_SR_TXE      (1<<5)  /**< @brief Transmission errro (only set when running as slave device) */
#define SPI_SR_RFF      (1<<4)  /**< @brief Receive  FIFO full */
#define SPI_SR_RFNE     (1<<3)  /**< @brief Receive  FIFO Not Emtpty */
#define SPI_SR_TFE      (1<<2)  /**< @brief Transmit FIFO Empty */
#define SPI_SR_TFNF     (1<<1)  /**< @brief Transmit FIFO Not Full */
#define SPI_SR_BUSY     (1<<0)  /**< @brief Serial Transfer in progress (1) or not (0) */

//
// SPI controller DMA control register Register (SPI 0 bus only, offset 0x4C)
//
#define SPI_DMACR_TDMAE (1<<1)  /**< @brief Transmit DMA enable */
#define SPI_DMACR_RDMAE (1<<0)  /**< @brief Receive  DMA enable */

/** @brief SPI software driver control block */
typedef struct tagSPIDRVCTX
{
    UINT32 frameSize;      /* CTRL0 3:0 - 0x07: 8bits*/
    UINT32 frameFormat;    /* CTRL0 5:4 - 0x0: Motorola SPI */
    UINT32 clkMode;        /* CTRL0 7:6 - 0x03 Start at first edge, high when inactive*/
    UINT32 transferMode;   /* CTRL0 9:8 - 0x0: rx&tx*/
    UINT32 frqDivider;     /* Divisor to setup speed to run SPI bus based on expansion bus clock */
    UINT32 BusID;          /* SPI Bus ID */
} SPICONFIG, *PSPICONFIG;

/** @brief SPI driver transfer (read and write) descriptor structure */
typedef struct
{
    UINT8* command_ptr;                    /**< @brief Command data (first n bytes sent of SPI message) */
    UINT8* tx_data_ptr;                    /**< @brief Pointer to data to transmit (if any) after command cycle completed, NULL if nothing to TX */
    UINT8* rx_data_ptr;                    /**< @brief Pointer to data to receive  (if any) after command cycle completed, NULL if nothing to RX */
    UINT32 cmd_size;                       /**< @brief Number of bytes for this command (includes any address bytes) */
    UINT32 data_size;                      /**< @brief Amount of data to send after command cycle */
//    UINT8  cs;                           /**< @brief Chip select (also known as "slave select (SS)) */

}S_SPI_TRANSFER_DES, *P_S_SPI_TRANSFER_DES;

//
// SPI control registers (where register is based on default low Speed SPI bus
//
#define SPI_CTRLR0_REG(BASEADDR)      (BASEADDR+0x00)
#define SPI_CTRLR1_REG(BASEADDR)      (BASEADDR+0x04)
#define SPI_SSIENR_REG(BASEADDR)      (BASEADDR+0x08)
#define SPI_MWCR_REG(BASEADDR)        (BASEADDR+0x0c)
#define SPI_SER_REG(BASEADDR)         (BASEADDR+0x10)
#define SPI_BAUDR_REG(BASEADDR)       (BASEADDR+0x14)
#define SPI_TXFTLR_REG(BASEADDR)      (BASEADDR+0x18)
#define SPI_RXFTLR_REG(BASEADDR)      (BASEADDR+0x1c)
#define SPI_TXFLR_REG(BASEADDR)       (BASEADDR+0x20)
#define SPI_RXFLR_REG(BASEADDR)       (BASEADDR+0x24)
#define SPI_SR_REG(BASEADDR)          (BASEADDR+0x28)
#define SPI_IMR_REG(BASEADDR)         (BASEADDR+0x2c)
#define SPI_ISR_REG(BASEADDR)         (BASEADDR+0x30)
#define SPI_RISR_REG(BASEADDR)        (BASEADDR+0x34)
#define SPI_TXOICR_REG(BASEADDR)      (BASEADDR+0x38)
#define SPI_RXOICR_REG(BASEADDR)      (BASEADDR+0x3c)
#define SPI_RXUICR_REG(BASEADDR)      (BASEADDR+0x40)
#define SPI_MSTICR_REG(BASEADDR)      (BASEADDR+0x44)
#define SPI_ICR_REG(BASEADDR)         (BASEADDR+0x48)
#define SPI_IDR_REG(BASEADDR)         (BASEADDR+0x58)
#define SPI_DR_REG(BASEADDR)          (BASEADDR+0x60)

/** @brief SPI controller register structure */
typedef struct
{
    VUINT32 CTRLR0;          /**< @brief Offset 0x00: Control register 0 */
    VUINT32 CTRLR1;          /**< @brief Offset 0x04: Control register 1 */
    VUINT32 SSIENR;          /**< @brief Offset 0x08: Slave enable register */
    VUINT32 MWCR;            /**< @brief Offset 0x0c: Microwire control register */
    VUINT32 SER;             /**< @brief Offset 0x10: Slave Select (SSn) register, 1 bit per slave selct */
    VUINT32 BAUDR;           /**< @brief Offset 0x14: Baud Rate register (based on expansion bus clock), (NOTE: Least significant bit ignored, always 0) */
    VUINT32 TXFTLR;          /**< @brief Offset 0x18: Transmit FIFO threshold register */
    VUINT32 RXFTLR;          /**< @brief Offset 0x1c: Receive  FIFO threshold register */
    VUINT16 TXFLR;           /**< @brief Offset 0x20: Transmit FIFO level     register (0 is empty) */
    VUINT16 RESERVED_22;
    VUINT16 RXFLR;           /**< @brief Offset 0x24: Receive  FIFO level     register (0 is empty) */
    VUINT16 RESERVED_26;
    VUINT32 SR;              /**< @brief Offset 0x28: Status register */
    VUINT32 IMR;             /**< @brief Offset 0x2c: Interrupt Mask register (0 for interrupt disable) */
    VUINT32 ISR;             /**< @brief Offset 0x30: Interrupt Status register */
    VUINT32 RISR;            /**< @brief Offset 0x34: Raw Interrupt Status register */
    VUINT32 TXOICR;          /**< @brief Offset 0x38: Transmit FIFO Overflow  Interrupt Clear register */
    VUINT32 RXOICR;          /**< @brief Offset 0x3c: Receive  FIFO Overflow  Interrupt Clear register */
    VUINT32 RXUICR;          /**< @brief Offset 0x40: Receive  FIFO Underflow Interrupt Clear register  */
    VUINT32 MSTICR;          /**< @brief Offset 0x44: Multi-Master Interrupt Clear Register */
    VUINT32 ICR;             /**< @brief Offset 0x48: Interrupt Clear Register */
//
// Registers for SPI0 bus (high speed SPI with DMA)
// Registers 0x4C, 0x50 and 0x54
//
// Note: These registers are not present for SPI bus 1 as that SPI
// controller does not have DMA capability
//
    VUINT32 DMACR;           /**< @brief Offset 0x4C: DMA control register (enable for TX and RX FIFO) */
    VUINT32 DMATDLR;         /**< @brief Offset 0x50: DMA transmit data level register */
    VUINT32 DMARDLR;         /**< @brief Offset 0x54: DMA receive  data level register */

    VUINT32 IDR;             /**< @brief Offset 0x58: Identification register */

    VUINT32 SSI_COMP_VERSION;/**< @brief Offset 0x5C: coreKit version ID register */

    VUINT16 DR;              /**< @brief Offset 0x60: Data Register (write for TX data, read for RX data) */
    VUINT16 RESERVED_62;
    VUINT32 RESERVED_64[35]; /**< @brief Offset 0x64-0xEF: Reserved */

    VUINT32 RX_SAMPLE_DLY;   /**< @brief Offset 0xF0: Receive Sample Delay Register */
    VUINT32 RSVD_0;          /**< @brief Offset 0xF4: Reserved location for possible future use */
    VUINT32 RSVD_1;          /**< @brief Offset 0xF8: Reserved location for possible future use */
    VUINT32 RSVD_2;          /**< @brief Offset 0xFC: Reserved location for possible future use */


} SPI_REGISTER_STRUCT, *P_SPI_REGISTER_STRUCT;

/**
	@brief This function is initializing SPI driver for specified chip and platform

    @param PSPIDCONFIG    pSpiConfig         [in]    -    Pointer to SPI configuration structure to get parameters for initialization

    @return [UINT32] error code SPI_RC_XXX
*/
UINT32 SPIDrvInit(PSPICONFIG pSpiConfig);

UINT16 SPIDrvWrite       (UINT32 busID, UINT32 cs, UINT16 data);
UINT16 SPIDrvRead        (UINT32 busID, UINT32 cs);
void   SPIDrvEnableCS    (UINT32 busID, UINT32 cs);
void   SPIDrvDisableCS   (UINT32 busID, UINT32 cs);
void   SPIDrvSetClockMode(UINT32 busID, UINT32 mode);
#ifdef USE_GPIO_CTRL_CS
void   SPISendByte       (UINT32 busID, UINT8 Data);
UINT8  SPISendReadByte   (UINT32 busID, UINT8 Data);
#endif

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
UINT32 SpiDrvReadWriteReg(UINT32 busID, UINT32 chipSelect, UINT16 RegisterAddress, UINT8 Data);

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
void SpiDrvWriteReg(UINT32 busID, UINT32 chipSelect, UINT16 RegisterAddress, UINT8 Data);

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
UINT8 SpiDrvReadReg(UINT32 busID, UINT32 chipSelect, UINT16 RegisterAddress);

UINT32 SPIDrvTransfer(UINT32 busID, UINT32 cs, S_SPI_TRANSFER_DES * ptrans_des);

#endif
