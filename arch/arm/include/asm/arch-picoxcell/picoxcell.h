/*****************************************************************************
 * $picoChipHeaderSubst$
 *****************************************************************************/

/*!
* \file picoxcell.h
* \brief Definitions for the picoxcell ARM sub-system.
*
* Copyright (c) 2006-2011 Picochip Ltd
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
* All enquiries to support@picochip.com
*/

#ifndef __PICOXCELL_H__
#define __PICOXCELL_H__

/*****************************************************************************/
/* Internal Boot ROM                                                         */
/*****************************************************************************/
#define PICOXCELL_BOOT_ROM_BASE         (0xFFFF0000)

/*****************************************************************************/
/* AXI2PICO Buffers                                                          */
/*****************************************************************************/
#define PICOXCELL_AXI2PICO_BUFFERS_BASE (0xC0000000)

/*****************************************************************************/
/* Peripheral Bus                                                            */
/*****************************************************************************/
#define PICOXCELL_MEMIF_BASE            (0x80000000)
#define PICOXCELL_EBI_BASE              (0x80010000)
#define PICOXCELL_EMAC_BASE             (0x80030000)
#define PICOXCELL_DMAC1_BASE            (0x80040000)
#define PICOXCELL_DMAC2_BASE            (0x80050000)
#define PICOXCELL_VIC0_BASE             (0x80060000)
#define PICOXCELL_VIC1_BASE             (0x80064000)
#define PICOXCELL_TZIC_BASE             (0x80068000)
#define PICOXCELL_TZPC_BASE             (0x80070000)
#define PICOXCELL_FUSE_BASE             (0x80080000)
#define PICOXCELL_SSI_BASE              (0x80090000)
#define PICOXCELL_AXI2CFG_BASE          (0x800A0000)
#define PICOXCELL_IPSEC_BASE            (0x80100000)
#define PICOXCELL_SRTP_BASE             (0x80140000)
#define PICOXCELL_CIPHER_BASE           (0x80180000)
#define PICOXCELL_RTCLK_BASE            (0x80200000)
#define PICOXCELL_TIMER_BASE            (0x80210000)
#define PICOXCELL_GPIO_BASE             (0x80220000)
#define PICOXCELL_UART1_BASE            (0x80230000)
#define PICOXCELL_UART2_BASE            (0x80240000)
#define PICOXCELL_WDOG_BASE             (0x80250000)

/*****************************************************************************/
/* External Memory                                                           */
/*****************************************************************************/
#define PICOXCELL_DDRBANK_BASE          (0x00000000)

#define PICOXCELL_EBI_CS0_BASE          (0x40000000)
#define PICOXCELL_EBI_CS1_BASE          (0x48000000)
#define PICOXCELL_EBI_CS2_BASE          (0x50000000)
#define PICOXCELL_EBI_CS3_BASE          (0x58000000)

#define PICOXCELL_FLASH_BASE            (PICOXCELL_EBI_CS0_BASE)

/*****************************************************************************/
/* Internal SRAM Memory                                                      */
/*****************************************************************************/
#define PICOXCELL_SRAM_BASE             (0x20000000)
#define PICOXCELL_SRAM_SIZE             (0x00020000)

/*****************************************************************************/
/* Silicon Revision                                                          */
/*****************************************************************************/
#define PC3X2_REV_A                 (0)
#define PC3X2_REV_D                 (1)

/*****************************************************************************/
/* Device Ids                                                                */
/*****************************************************************************/
#define PC302_DEVICE_ID             (0x03)
#define PC312_DEVICE_ID             (0x07)
#define PC313_DEVICE_ID             (0x20)
#define PC323_DEVICE_ID             (0x21)
#define PC333_DEVICE_ID             (0x22)
#define PC3008_DEVICE_ID            (0x30)

#endif /* __PICOXCELL_H__ */
