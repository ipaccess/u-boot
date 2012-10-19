/*****************************************************************************
 * $picoChipHeaderSubst$
 *****************************************************************************/

/*!
* \file picochippc73032nor.h
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

#ifndef __CONFIG_PC73032_NOR__
#define __CONFIG_PC73032_NOR__

#include <asm/arch/picoxcell.h>
#include <asm/sizes.h>
/* Start address of the flash memory */
#define CONFIG_SYS_FLASH_BASE   (PICOXCELL_FLASH_BASE)

/* Define Flash memory sector size */
#define FLASH_SECTOR_SIZE	(SZ_128K)

/* Maximum number of memory banks (devices) */
#define CONFIG_SYS_MAX_FLASH_BANKS  (1)

/* Maximum number of sectors per flash device */
#define CONFIG_SYS_MAX_FLASH_SECT   (1024)

/* Timeouts for Flash Erasing and writing */
#define CONFIG_SYS_FLASH_ERASE_TOUT (2 * CONFIG_SYS_HZ)
#define CONFIG_SYS_FLASH_WRITE_TOUT (2 * CONFIG_SYS_HZ)

/* Build the cfi mtd driver */
#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_SYS_FLASH_CFI

/* print 'E' for empty sector on flinfo */
#define CONFIG_SYS_FLASH_EMPTY_INFO

/* U-Boot lives in the bottom of the Flash memory */
#define CONFIG_SYS_MONITOR_BASE (CONFIG_SYS_FLASH_BASE)

/* U-Boot occupies 2 flash sectors */
#define CONFIG_SYS_MONITOR_LEN  (2 * FLASH_SECTOR_SIZE)

/* Environment variables stored in Flash memory */
#define CONFIG_ENV_IS_IN_FLASH

/* Start address of environment variables */
#define CONFIG_ENV_ADDR         (CONFIG_SYS_MONITOR_BASE + CONFIG_SYS_MONITOR_LEN)

/* One flash sector for environment varisbles */
#define CONFIG_ENV_SECT_SIZE    (FLASH_SECTOR_SIZE)

/* But 64 KBytes is sufficient */
#define CONFIG_ENV_SIZE		(SZ_64K)

/* Turn off wite protection for vendor parameters */
#define CONFIG_ENV_OVERWRITE

#ifdef CONFIG_RUN_FROM_RAM
#define CONFIG_SKIP_LOWLEVEL_INIT
#define CONFIG_SYS_TEXT_BASE        0x06000000
#else
#define CONFIG_SYS_TEXT_BASE        0x40000000
#endif /* CONFIG_RUN_FROM_RAM */

/* Default command line mtd partitioning */
#define MTD_PARTITION_DEFAULT   "nor0,0"

#define MTDIDS_DEFAULT          "nor0=physmap-flash"

#define MTDPARTS_DEFAULT	"mtdparts=physmap-flash:"\
                                "256K(Boot),"\
                                "128K(BootEnvironment),"\
                                "4M(Kernel),"\
                                "-(Application)"

#include <configs/picochippc73032-common.h>

#endif /* __CONFIG_PC73032_NOR__ */
