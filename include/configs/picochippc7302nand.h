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
 *       board/picochip/pc7302/nand.c about gpio pins used
 *       and Picoxcell booting modes before defining CONFIG_CMD_NAND
 */

/* Needed for nand_util.c */
#define CONFIG_SYS_64BIT_VSPRINTF

/* Do not perform any low level initialisation */
#define CONFIG_SKIP_LOWLEVEL_INIT

/* No flash memory in the system */
#define CONFIG_SYS_NO_FLASH

/* We are runing from ram with NAND support */

/* NAND Flash memory map
 *
 *  Block 0-7   Reserved
 *  Block 8-11  U-Boot image
 *  Block 12-15 Reserved
 *  Block 16-19 Redundant U-Boot image
 *  Block 20-23 Reserved
 *  Block 24    U-Boot env variables
 *  Block 25    Redundant U-Boot env variables
 *  Block 26-27 Reserved
 *  Block 28    Linux kernel...
 *
 */
#define CONFIG_ENV_IS_IN_NAND

/* Offset address of environment variables */
#define CONFIG_ENV_OFFSET           (NAND_FLASH_SECTOR_SIZE * 24)

/* Size of environment variables */
#define CONFIG_ENV_SIZE             (NAND_FLASH_SECTOR_SIZE)

/* Offset address of the redundant envinronment variables */
#define CONFIG_ENV_OFFSET_REDUND    (NAND_FLASH_SECTOR_SIZE * 25)

/* Turn off wite protection for vendor parameters */
#define CONFIG_ENV_OVERWRITE

#define CONFIG_CMD_NAND

#define CONFIG_USE_UBIFS

/* Build U-Boot to run at this address */
#define CONFIG_SYS_TEXT_BASE    0x06000000

/* Default command line mtd partitioning */
#define MTD_PARTITION_DEFAULT   "nand0,0"

#define MTDIDS_DEFAULT          "nand0=gpio-nand"

#define MTDPARTS_DEFAULT	"mtdparts=gpio-nand:"\
                                "1M@1M(Boot),"\
				"1M(RedundantBoot),"\
				"128K(BootEnvironment),"\
                                "128K(RedundantBootEnv),"\
                                "8M@0x380000(KernelA),"\
                                "80M(FileSystemA),"\
                                "8M(Configuration),"\
                                "8M(KernelB),"\
                                "80M(FileSystemB)"

#include <configs/picochippc7302-common.h>

#endif /* __CONFIG_PC7302_NAND__ */
