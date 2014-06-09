/*****************************************************************************
 * $picoChipHeaderSubst$
 *****************************************************************************/

/*!
* \file spi.h
* \brief Definitions for the picoxcell spi block.
*
* Copyright (c) 2006-2011 Picochip Ltd
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
* All enquiries to support@picochip.com
*/

#ifndef __PICOXCELL_SPI_H__
#define __PICOXCELL_SPI_H__

/* Constants --------------------------------------------------------------- */

/*****************************************************************************/
/* Register Offset Addresses                                                 */
/*****************************************************************************/

/* Functional Registers */

#define SSI_CTRL_REG_0_REG_OFFSET                   (0x00)
#define SSI_CTRL_REG_1_REG_OFFSET                   (0x04)
#define SSI_ENABLE_REG_REG_OFFSET                   (0x08)
#define SSI_MW_CTRL_REG_OFFSET                      (0x0C)
#define SSI_SLAVE_ENABLE_REG_OFFSET                 (0x10)
#define SSI_BAUD_RATE_SEL_REG_OFFSET                (0x14)
#define SSI_TX_FIFO_THRESHOLD_REG_OFFSET            (0x18)
#define SSI_RX_FIFO_THRESHOLD_REG_OFFSET            (0x1C)
#define SSI_TX_FIFO_LEVEL_REG_OFFSET                (0x20)
#define SSI_RX_FIFO_LEVEL_REG_OFFSET                (0x24)
#define SSI_STATUS_REG_OFFSET                       (0x28)
#define SSI_IMR_REG_OFFSET                          (0x2C)
#define SSI_ISR_REG_OFFSET                          (0x30)
#define SSI_RAW_ISR_REG_OFFSET                      (0x34)
#define SSI_TX_FIFO_OVRFLOW_INT_CLEAR_REG_OFFSET    (0x38)
#define SSI_RX_FIFO_OVRFLOW_INT_CLEAR_REG_OFFSET    (0x3C)
#define SSI_RX_FIFO_UNDFLOW_INT_CLEAR_REG_OFFSET    (0x40)
#define SSI_MM_INT_CLEAR_REG_OFFSET                 (0x44)
#define SSI_INT_CLEAR_REG_OFFSET                    (0x48)
#define SSI_DMA_CTRL_REG_OFFSET                     (0x4C)
#define SSI_DMA_TX_DATA_LEVEL_REG_OFFSET            (0x50)
#define SSI_DMA_RX_DATA_LEVEL_REG_OFFSET            (0x54)
#define SSI_DATA_REG_OFFSET                         (0x60)

/* Identification Registers */

#define SSI_ID_REG_OFFSET                           (0x58)
#define SSI_COMP_VERSION_REG_OFFSET                 (0x5C)

/* Types ------------------------------------------------------------------- */

/* Macros ------------------------------------------------------------------ */
#define PICOXCELL_MAX_NUMBER_SPI_CS     (4)
#define PICOXCELL_MAX_NUMBER_SPI_BUSSES (1)
#define PICOXCELL_MIN_SPI_CLK_DIVIDER   (2)
#define PICOXCELL_MAX_SPI_CLK_DIVIDER   (65534)

/* SSI_CTRL_REG_0_REG_OFFSET bites */
#define PICOXCELL_SPI_LOOPBACK_MODE     (1 << 11)
#define PICOXCELL_SPI_NORMAL_MODE       (0)
#define PICOXCELL_SPI_TMOD_TX_RX        (0x0)
#define PICOXCELL_SPI_TMOD_TX           (0x1 << 8)
#define PICOXCELL_SPI_TMOD_RX           (0x2 << 8)
#define PICOXCELL_SPI_TMOD_EEPROM_RX    (0x3 << 8)
#define PICOXCELL_SPI_SCPOL             (1 << 7)
#define PICOXCELL_SPI_SCPH              (1 << 6)
#define PICOXCELL_SPI_MOTO_FORMAT       (0x0)
#define PICOXCELL_SPI_DATA_FRM_8_BIT    (0x7)

/* SSI_ENABLE_REG_REG_OFFSET bits */
#define PICOXCELL_SPI_ENABLE            (1)
#define PICOXCELL_SPI_DISABLE           (0)

/* SSI_SLAVE_ENABLE_REG_OFFSET bits */
#define PICOXCELL_SPI_SLAVES_DISABLE    (0)

/* SSI_STATUS_REG_OFFSET bits */
#define PICOXCELL_SPI_STATUS_DCOL       (1 << 6)
#define PICOXCELL_SPI_STATUS_TXE        (1 << 5)
#define PICOXCELL_SPI_STATUS_RFF        (1 << 4)
#define PICOXCELL_SPI_STATUS_RFNE       (1 << 3)
#define PICOXCELL_SPI_STATUS_TFE        (1 << 2)
#define PICOXCELL_SPI_STATUS_TFNF       (1 << 1)
#define PICOXCELL_SPI_STATUS_BUSY       (1 << 0)

/* SSI_IMR_REG_RESET bits */
#define PICOXCELL_SPI_MASK_ALL_INTS     (0xFFFF)

#endif /* __PICOXCELL_SPI_H__ */
