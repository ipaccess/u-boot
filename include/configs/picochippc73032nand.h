/*****************************************************************************
 * $picoChipHeaderSubst$
 *****************************************************************************/

/*!
* \file picochippc73032nand.h
* \brief Configuration file for U-Boot on the PC73032 platform.
*
* Copyright (c) 2012 Picochip Ltd
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
* All enquiries to support@picochip.com
*/

#ifndef __CONFIG_PC73032_NAND__
#define __CONFIG_PC73032_NAND__

#include <asm/arch/picoxcell.h>
#include <asm/sizes.h>

/*-----------------------------------------------------------------------------
 * High Level Configuration Options
 */

/* Build U-Boot to run at this address */
#define CONFIG_SYS_TEXT_BASE        0x06000000

/* Do not perform any low level initialisation
 *
 * Note: We will be running from ram and U-Boot
 *       will have been loaded by a first stage bootloader
 */
#define CONFIG_SKIP_LOWLEVEL_INIT

/*-----------------------------------------------------------------------------
 * NAND Flash Memory Stuff
 */
#define CONFIG_NAND_DENALI

#define CONFIG_SYS_NAND_BASE        (PICOXCELL_EBI_CS2_BASE)
#define CONFIG_SYS_NAND_MAX_CHIPS   (1)
#define CONFIG_SYS_MAX_NAND_DEVICE  (1)
#define CONFIG_SYS_NAND_ONFI_DETECTION  (1)

/* Define Flash memory sector size */
#define NAND_FLASH_SECTOR_SIZE      (SZ_128K)

/* Needed for nand_util.c */
#define CONFIG_SYS_64BIT_VSPRINTF

/* No flash memory in the system */
#define CONFIG_SYS_NO_FLASH

/*-----------------------------------------------------------------------------
 * U-Boot Environment Stuff
 */

/* NAND Flash memory map
 *
 *  Block 0     spinand first stage bootloader #0
 *  Block 1     spinand first stage bootloader #1
 *  Block 2     spinand first stage bootloader #2
 *  Block 3     spinand first stage bootloader #3
 *  Block 4-7   Reserved
 *  Block 8-15  U-Boot image
 *  Block 16-23 Redundant U-Boot image
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

/* Default command line mtd partitioning */
#define MTD_PARTITION_DEFAULT   "nand0,0"

#define MTDIDS_DEFAULT          "nand0=denali-nand"

#define MTDPARTS_DEFAULT	"mtdparts=denali-nand:"\
                                "128K(NandFirstStage0),"\
                                "128K(NandFirstStage1),"\
                                "128K(NandFirstStage2),"\
                                "128K(NandFirstStage3),"\
                                "1M@1M(Boot),"\
				"1M(RedundantBoot),"\
				"128K(BootEnvironment),"\
                                "128K(RedundantBootEnv),"\
                                "8M@0x380000(KernelA),"\
                                "80M(FileSystemA),"\
                                "8M(Configuration),"\
                                "8M(KernelB),"\
                                "80M(FileSystemB)"

#include <configs/picochippc73032-common.h>

#endif /* __CONFIG_PC73032_NAND__ */
