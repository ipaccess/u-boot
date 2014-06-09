/*****************************************************************************
 * $picoChipHeaderSubst$
 *****************************************************************************/

/*!
* \file picochippc7302.h
* \brief Configuration file for U-Boot on the PC7302 platform.
*
* Copyright (c) 2006-2011 Picochip Ltd
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
* All enquiries to support@picochip.com
*/

#ifndef __CONFIG_PC7302_NAND__
#define __CONFIG_PC7302_NAND__

#include <asm/arch/picoxcell.h>
#include <asm/sizes.h>

/*-----------------------------------------------------------------------------
 * NAND Flash Memory Stuff
 */
#define CONFIG_SYS_NAND_BASE        (PICOXCELL_EBI_CS2_BASE)
#define CONFIG_SYS_NAND_MAX_CHIPS   (1)
#define CONFIG_SYS_MAX_NAND_DEVICE  (1)

/* Define Flash memory sector size */
#define NAND_FLASH_SECTOR_SIZE      (SZ_128K)

/* Include support / commands for NAND Flash
 *
 * Note: Please read the comments in file
 *       board/picochip/pc7302/mt29f2g08aadwp.c about gpio pins used
 *       and PC302 booting modes before defining CONFIG_CMD_NAND
 */

/* Needed for nand_util.c */
#define CONFIG_SYS_64BIT_VSPRINTF

/* Do not perform any low level initialisation */
#define CONFIG_SKIP_LOWLEVEL_INIT   1

/* No flash memory in the system */
#define CONFIG_SYS_NO_FLASH

/* We are runing from ram with NAND support */

/* NAND Flash memory map
 *
 *  Block 0-3 U-Boot image
 *  Block 4-7 Redundant U-Boot image
 *  Block 8 U-Boot environment
 *  Block 9 Redundant U-Boot environment
 *  Block 10 Not Used
 *  Block 11 Not Used
 *  Block 12 Linux kernel
 *
 */
#define CONFIG_ENV_IS_IN_NAND

/* Offset address of environment variables */
#define CONFIG_ENV_OFFSET           (NAND_FLASH_SECTOR_SIZE * 8)

/* Size of environment variables */
#define CONFIG_ENV_SIZE             (NAND_FLASH_SECTOR_SIZE)

/* Offset address of the redundant envinronment variables */
#define CONFIG_ENV_OFFSET_REDUND    (NAND_FLASH_SECTOR_SIZE * 9)

/* Turn off wite protection for vendor parameters */
#define CONFIG_ENV_OVERWRITE

#define CONFIG_CMD_NAND

#define CONFIG_USE_UBIFS

/* Build U-Boot to run at this address */
#define CONFIG_SYS_TEXT_BASE    0x06000000

#include <configs/picochippc7302-common.h>

#endif /* __CONFIG_PC7302_NAND__ */
