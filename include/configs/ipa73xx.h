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

#ifndef __CONFIG_H
#define __CONFIG_H

#include <asm/arch/picoxcell.h>
#include <asm/sizes.h>
#include <asm/arch-picoxcell/uart.h>
/* #define DEBUG */

/*
 * SPI flash stuff
 */
#define CONFIG_DW_SPI
#define CONFIG_SPI_FLASH
#define CONFIG_SPI_FLASH_STMICRO

#define CONFIG_SF_DEFAULT_BUS 0
#define CONFIG_SF_DEFAULT_CS 0
#define CONFIG_SF_DEFAULT_MODE SPI_MODE_3 /* include/spi.h */
#define CONFIG_SF_DEFAULT_SPEED 2000000

#define CONFIG_SYS_NO_FLASH

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

/* As we might load u-boot over JTAG we would like to perform low level initialisation */
#define CONFIG_SKIP_LOWLEVEL_INIT

/* No flash memory in the system */


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
#define CONFIG_ENV_OFFSET           (NAND_FLASH_SECTOR_SIZE * 39)

/* Size of environment variables */
#define CONFIG_ENV_SIZE             (NAND_FLASH_SECTOR_SIZE)

/* Offset address of the redundant envinronment variables */
#define CONFIG_ENV_OFFSET_REDUND    (NAND_FLASH_SECTOR_SIZE * 71)

/* Turn off wite protection for vendor parameters */
#define CONFIG_ENV_OVERWRITE

#define CONFIG_USE_UBI

#define CONFIG_USE_UBIFS

/* Build U-Boot to run at this address */
#define CONFIG_SYS_TEXT_BASE    0x06000000

/* Default command line mtd partitioning */
#define MTD_PARTITION_DEFAULT   "nand0,0"

#define MTDIDS_DEFAULT          "nand0=gpio-nand"

#define MTDPARTS_DEFAULT	"mtdparts=gpio-nand:256K@0K(SPL0),"	\
						   "256K@256K(SPL1),"	\
						   "256K@512K(SPL2),"	\
						   "256K@768K(SPL3),"	\
						   "4M@1M(UBOOT0),"	\
						   "4M@5M(UBOOT1),"	\
						   "-@9M(FS)"

#define LINUX_CONSOLEDEV "ttyS0"
#define CMD_LINE_ARGS_LINUX "rdinit=/init console=" LINUX_CONSOLEDEV ","  __stringify(CONFIG_BAUDRATE) "n8 elevator=noop " MTDPARTS_DEFAULT
#define CMD_LINE_ARGS_LINUX_SILENT "rdinit=/init console=tty0 elevator=noop " MTDPARTS_DEFAULT
                           
                           
/*-----------------------------------------------------------------------------
 * Platform Identification Stuff
 */
#define PICOCHIP "ip.access"

/* Which hardware platform I am destined for */
#define PICOCHIP_PLATFORM "ipapx3xx"

/* Specific version of this build */
#ifndef PICOCHIP_PLATFORM_VERSION
/* a dummy value */
#define PICOCHIP_PLATFORM_VERSION "42"
#endif /* PICOCHIP_PLATFORM_VERSION */

#define CONFIG_IDENT_STRING " "PICOCHIP"-"PICOCHIP_PLATFORM_VERSION \
                            "-"PICOCHIP_PLATFORM

/*-----------------------------------------------------------------------------
 * High Level Configuration Options
 */
/* Running on Picochip PC3X2 Silicon */
#define CONFIG_PICOCHIP_PC3X2

/* Running on Picochip PC3X3 Silicon */
#define CONFIG_PICOCHIP_PC3X3

/* Running on a Picochip PC7302 platform */
#define CONFIG_PICOCHIP_PC7302

/* Base address of the onchip SRAM */
#define PICOXCELL_ONCHIP_SRAM_BASE      (PICOXCELL_SRAM_BASE)
#define PICOXCELL_ONCHIP_SRAM_SIZE      (PICOXCELL_SRAM_SIZE)

/* Bootable Flash memory has to live here (/ebi_decode0) */
#define PICOXCELL_BOOTABLE_FLASH_BASE   (PICOXCELL_FLASH_BASE)

/* ARM Sub-system peripherals are clocked at 200MHz */
#define PICOXCELL_AHB_CLOCK_FREQ        (200000000)

/* Don't use Interrupts */
#undef CONFIG_USE_IRQ

/* A time tick is 1 millisecond (this is NOT CPU tick) */
#define CONFIG_SYS_HZ               (1000)

/* Display board info */
#undef CONFIG_DISPLAY_BOARDINFO

/* Display cpu info */
#undef CONFIG_DISPLAY_CPUINFO

/* Do not reboot the platform on a panic */
#define CONFIG_PANIC_HANG

/*-----------------------------------------------------------------------
 * Stack Sizes
 *
 * The stack sizes are set up in arch/arm/lib/board.c using the
 * settings below
 */
#define CONFIG_STACKSIZE	(SZ_256K) 	    /* regular stack */
#ifdef CONFIG_USE_IRQ
#define CONFIG_STACKSIZE_IRQ	(SZ_4K)             /* IRQ stack */
#define CONFIG_STACKSIZE_FIQ	(SZ_4K)    	    /* FIQ stack */
#endif /* CONFIG_USE_IRQ */

/*-----------------------------------------------------------------------
 * Initial Stack Pointer
 *
 * Note: This is only used before U-Boot relocation takes place.
 *       The global data is stored above this address, whilst the stack
 *       is below this address. After relocation the stack is automatically
 *       moved to the top of the available sdram.
 */
#define CONFIG_SYS_INIT_SP_ADDR (0x00001000)

/*-----------------------------------------------------------------------------
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN   (8*SZ_1M)

/*-----------------------------------------------------------------------------
 * Linux Kernel Stuff
 */
/* Allow passing of command line args (bootargs) to the linux kernel*/
#define CONFIG_CMDLINE_TAG          1
#define CONFIG_SETUP_MEMORY_TAGS    1
#define CONFIG_INITRD_TAG           1

/*-----------------------------------------------------------------------------
 * DDR2 RAM Memory Map
 *
 * We have 1 linear addressable RAM bank
 *
 * Note: CONFIG_SYS_SDRAM_BASE must start at 0
 */
#define CONFIG_SYS_SDRAM_BASE	(0x00000000)
#define CONFIG_NR_DRAM_BANKS    (1)
#define PHYS_SDRAM_1		(PICOXCELL_DDRBANK_BASE)
#define PHYS_SDRAM_1_SIZE	(SZ_128M)

/*-----------------------------------------------------------------------------
 * Timer Stuff
 */
#define CONFIG_SYS_TIMERBASE    (PICOXCELL_TIMER_BASE)

/*-----------------------------------------------------------------------------
 * Ethernet Stuff
 */
#define CONFIG_MACB
#define CONFIG_PHY_ADDR         (0)
#define CONFIG_NET_MULTI

/*-----------------------------------------------------------------------------
 * MTD Support
 */
#define CONFIG_MTD_DEVICE
#define CONFIG_CMD_MTDPARTS
/* Enable command line MTD partitioning */
#define CONFIG_MTD_PARTITIONS
/*-----------------------------------------------------------------------------
 * UBI Support
 */
#define CONFIG_CMD_UBI
#define CONFIG_UBI_SILENCE_MSG
#define CONFIG_LZO
#define CONFIG_RBTREE

/* fitImage location in UBIFS*/
#define UBI_PART "FS"
#define UBI_VOLUME "ubi0"
#define UBIFS_0 "fs0"
#define UBIFS_1 "fs1"
#define FIT_IMAGE "fitImage"

/*-----------------------------------------------------------------------------
 * Enable FIT and OF control support 
 */
#define CONFIG_OF_CONTROL
#define CONFIG_OF_EMBED
#define CONFIG_DEFAULT_DEVICE_TREE "ipa73xx"
#define CONFIG_OF_LIBFDT
#define CONFIG_FIT
#define CONFIG_FIT_SIGNATURE
#define CONFIG_RSA
#define CONFIG_FIT_REVOCATION
#define CONFIG_FIT_SIGNATURE_REQUIRED_KEYNODE_ONLY

/*-----------------------------------------------------------------------------
 * UBIFS Support
 */
#define CONFIG_CMD_UBIFS
#define CONFIG_UBIFS_SILENCE_MSG


/*-----------------------------------------------------------------------------
 * Serial Port Stuff
 */
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE (-4)
#define CONFIG_SYS_NS16550_CLK      (3686400)
#define CONFIG_SYS_NS16550_COM1     (PICOXCELL_UART1_BASE)

/* Our uart registers need 32 bit access */
#define CONFIG_SYS_NS16550_MEM32

/* Console on Uart #0 */
#define CONFIG_CONS_INDEX	    (1)

/* Table of supported baud rates */
#define CONFIG_SYS_BAUDRATE_TABLE   { 9600, 19200,  \
                                      38400, 57600, \
                                      115200, 230400 }

/*-----------------------------------------------------------------------------
 * U-Boot Memory Test (mtest command) Stuff
 */
#define CONFIG_CMD_MEMTEST

/* Default start address for memory test */
#define CONFIG_SYS_MEMTEST_START    (PICOXCELL_ONCHIP_SRAM_BASE)

/* Default end address for memory test */
#define CONFIG_SYS_MEMTEST_END  (CONFIG_SYS_MEMTEST_START + \
                                 PICOXCELL_ONCHIP_SRAM_SIZE - 1)

/* Define this to use the super duper memory test */
#define CONFIG_SYS_ALT_MEMTEST

/* Use Uart #1 scratch pad reg */
#define CONFIG_SYS_MEMTEST_SCRATCH  (PICOXCELL_UART1_BASE + \
                                     UART_SCRATCH_REG_OFFSET)

/*-----------------------------------------------------------------------------
 * U-Boot Supported Commands
 */
#include "config_cmd_default.h"

#define CONFIG_CMD_DHCP
/* #define CONFIG_CMD_SNTP */
#define CONFIG_CMD_DNS
#define CONFIG_CMD_PING
#define CONFIG_CMD_NAND
#define CONFIG_CMD_SF
#define CONFIG_CMD_BSP
#define CONFIG_CMD_SOURCE

/* Turn off a bunch of default commands */
#undef CONFIG_CMD_BOOTD
#undef CONFIG_CMD_CONSOLE

#undef CONFIG_CMD_EDITENV
#undef CONFIG_CMD_FPGA
#undef CONFIG_CMD_ITEST
#undef CONFIG_CMD_IMLS
#undef CONFIG_CMD_LOADB
#undef CONFIG_CMD_LOADS
#undef CONFIG_CMD_NFS
#undef CONFIG_CMD_SETGETDCR
#undef CONFIG_CMD_XIMG

#ifdef CONFIG_SYS_NO_FLASH
#undef CONFIG_CMD_FLASH
#endif /* CONFIG_SYS_NO_FLASH */

#ifdef CONFIG_ENV_IS_NOWHERE
#undef CONFIG_CMD_SAVEENV
#endif /* CONFIG_ENV_IS_NOWHERE */

/* Use the HUSH parser */
#define CONFIG_SYS_HUSH_PARSER

#ifdef  CONFIG_SYS_HUSH_PARSER
/* This defines the secondary prompt string */
#define CONFIG_SYS_PROMPT_HUSH_PS2 "> "
#endif /* CONFIG_SYS_HUSH_PARSER */

/* Enable command line editing and history */
#define CONFIG_CMDLINE_EDITING

#ifndef CONFIG_SYS_NO_FLASH
#define CONFIG_FLASH_CFI_MTD
#endif


/*-----------------------------------------------------------------------------
 * Miscellaneous Configurable Options...
 */
/* Use 'long' help messages */
#define CONFIG_SYS_LONGHELP

/* Monitor Command Prompt */
#define CONFIG_SYS_PROMPT   "ipapx3xx=> "

/* Console I/O Buffer Size*/
#define CONFIG_SYS_CBSIZE   (SZ_1K)

/* Print buffer size */
#define CONFIG_SYS_PBSIZE   (CONFIG_SYS_CBSIZE + \
                             sizeof(CONFIG_SYS_PROMPT) + 16)

/* Maximum number of command args */
#define CONFIG_SYS_MAXARGS  (16)

/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE (CONFIG_SYS_CBSIZE)

/* Default load address for tftp, bootm and friends */
#define CONFIG_SYS_LOAD_ADDR    0x02000000
#define CONFIG_LOADADDR         CONFIG_SYS_LOAD_ADDR

/*-----------------------------------------------------------------------
 * Environment Configuration
 */

#if defined(CONFIG_MACB)

/* Picochip OUI
 *
 * Note: The MAC Address defined by 'CONFIG_ETHADDR' is based on
 * Picochip's OUI,see http://standards.ieee.org/regauth/oui/index.shtml
 * for more information. It will need to be modified for each and every
 * individual hardware platform.
 */
#define CONFIG_ETHADDR          00:15:E1:00:00:00

/* Picochip default for testing, will need noodling by users */
#define CONFIG_IPADDR           192.168.0.201

#define CONFIG_HOSTNAME	        picopc7302
#define CONFIG_ROOTPATH	        "/var/nfspc7302"
#define CONFIG_BOOTFILE	        "kernel.bin"

#define CONFIG_SERVERIP         192.168.0.5
#define CONFIG_GATEWAYIP        192.168.0.5
#define CONFIG_NETMASK          255.255.255.0

/* Enable the MII utility commands */
#undef CONFIG_CMD_MII

#endif /* CONFIG_MACB */

/* This is the offset from the start of NAND Flash
 * to where the Linux kernel can be found.
 */
#define CONFIG_NAND_KERNEL_OFFSET  0x00380000

/* Time in seconds before autoboot, -1 disables auto-boot */
#define CONFIG_BOOTDELAY        3
#define CONFIG_AUTOBOOT_KEYED
#define CONFIG_AUTOBOOT_STOP_STR "stop"
#define CONFIG_AUTOBOOT_PROMPT "autoboot in %d seconds (\"stop\" to stop)\n",bootdelay

/* The boot command will set bootargs */
#undef  CONFIG_BOOTARGS

/* Default console baud rate */
#define CONFIG_BAUDRATE	        115200

/* Unless specified here we'll just rely on the kernel default */
#define OTHERBOOTARGS

/* Define the JFFS2 root filesystem partition (NOR Flash) */
#define NOR_JFFS2_ROOT          /dev/mtdblock3

/* Define the UBIFS root filesystem partition (NOR Flash) */
#define NOR_UBIFS_ROOT          3

/* Define the UBIFS root filesystem partition for NAND Flash */
#define NAND_UBIFS_ROOT         5

/* IPA-specific commands */
#define CONFIG_CMD_LEDSET
#define CONFIG_CMD_LEDC
#define CONFIG_CMD_SECPARM
#define CONFIG_CMD_LIE
#define CONFIG_CMD_KEY
#define CONFIG_CMD_CHARACTERISE_HW

/* Allow us to track boot progress */
#define CONFIG_SHOW_BOOT_PROGRESS  1

#define CONFIG_BOOTCOUNT_LIMIT 4

#define SECURE_BOOT_COMMAND                        \
    "ledc all green off 1 300;"                    \
    "setenv bootargs $bootargs $othbootargs;"      \
    "setenv bootargs $bootargs $bootbankargs;"     \
    "if secparm secboot; then"                     \
    "  if secparm devmode; then"                   \
    "    if key unrequire ipaoem0; then"           \
    "      if key unrequire tstoem0; then"         \
    "        if key unrequire manoem0; then"       \
    "          if key require dev conf; then"      \
    "            if bootm $loadaddr; then"         \
    "              ledc pwr green red 3 1000;"     \
    "              reset;"                         \
    "            fi;"                              \
    "          fi;"                                \
    "        fi;"                                  \
    "      fi;"                                    \
    "    fi;"                                      \
    "    if key unrequire ipaoem0; then"           \
    "      if key unrequire dev; then"             \
    "        if key unrequire manoem0; then"       \
    "          if key require tstoem0 conf; then"  \
    "            if bootm $loadaddr; then"         \
    "              ledc pwr green red 3 1000;"     \
    "              reset;"                         \
    "            fi;"                              \
    "          fi;"                                \
    "        fi;"                                  \
    "      fi;"                                    \
    "    fi;"                                      \
    "    if key unrequire tstoem0; then"           \
    "      if key unrequire dev; then"             \
    "        if key unrequire manoem0; then"       \
    "          if key require ipaoem0 conf; then"  \
    "            if bootm $loadaddr; then"         \
    "              ledc pwr green red 3 1000;"     \
    "              reset;"                         \
    "            fi;"                              \
    "          fi;"                                \
    "        fi;"                                  \
    "      fi;"                                    \
    "    fi;"                                      \
    "    if key unrequire tstoem0; then"           \
    "      if key unrequire dev; then"             \
    "        if key unrequire ipaoem0; then"       \
    "          if key require manoem0 conf; then"  \
    "            if bootm $loadaddr; then"         \
    "              ledc pwr green red 3 1000;"     \
    "              reset;"                         \
    "            fi;"                              \
    "          fi;"                                \
    "        fi;"                                  \
    "      fi;"                                    \
    "    fi;"                                      \
    "  else"                                       \
    "    if key unrequire dev; then"               \
    "      if key unrequire tstoem0; then"         \
    "        if key unrequire manoem0; then"       \
    "          if key require ipaoem0 conf; then"  \
    "            if bootm $loadaddr; then"         \
    "              ledc pwr green red 3 1000;"     \
    "              reset;"                         \
    "            fi;"                              \
    "          fi;"                                \
    "        fi;"                                  \
    "      fi;"                                    \
    "    fi;"                                      \
    "    if key unrequire dev; then"               \
    "      if key unrequire tstoem0; then"         \
    "        if key unrequire ipaoem0; then"       \
    "          if key require manoem0 conf; then"  \
    "            if bootm $loadaddr; then"         \
    "              ledc pwr green red 3 1000;"     \
    "              reset;"                         \
    "            fi;"                              \
    "          fi;"                                \
    "        fi;"                                  \
    "      fi;"                                    \
    "    fi;"                                      \
    "  fi;"                                        \
    "else"                                         \
    "  key unrequire dev;"                         \
    "  key unrequire tstoem0;"                     \
    "  key unrequire ipaoem0;"                     \
    "  key unrequire manoem0;"                     \
    "  bootm $loadaddr;"                           \
    "fi;"                                          \
    "ledc pwr green red 3 1000;"                   \
    "reset;"

#define TEST_BOOT_COUNT_EXCEEDED                                     \
    "if bootcount_combined_exceeded; then"                           \
    " run bootfailedhang;"                                           \
    "fi;"

#define SET_BOOTARGS                                                 \
    "if silent_mode_enabled; then"                                   \
    " setenv bootargs " CMD_LINE_ARGS_LINUX_SILENT ";"               \
    "else"                                                           \
    " setenv bootargs " CMD_LINE_ARGS_LINUX ";"                      \
    "fi;"                                                            \


#define UBIBOOT_COMMAND                                              \
    TEST_BOOT_COUNT_EXCEEDED                                         \
    SET_BOOTARGS                                                     \
    "setenv fsactive " UBIFS_1 ";"                                   \
    "setenv fsstandby " UBIFS_0 ";"                                  \
    "ubi part " UBI_PART ";"                                         \
    "ubifsmount " UBI_VOLUME ":$fsactive;"                           \
    "if ubifsload $loadaddr primary.flag; then"                      \
    " echo \"Normal boot from $fsactive\";"                          \
    "else"                                                           \
    " setenv fsactive " UBIFS_0 ";"                                  \
    " setenv fsstandby " UBIFS_1 ";"                                 \
    " ubifsmount " UBI_VOLUME ":$fsactive;"                          \
    " echo \"Normal boot from $fsactive\";"                          \
    "fi;"                                                            \
    "if ubifsload $loadaddr " FIT_IMAGE "; then"                     \
    " setenv bootbankargs fsactive=$fsactive fsstandby=$fsstandby;"  \
    " run secureboot;"                                               \
    "fi;"                                                            \
    "reset;"

#define UBIALTBOOT_COMMAND                                           \
    TEST_BOOT_COUNT_EXCEEDED                                         \
    SET_BOOTARGS                                                     \
    "setenv fsactive " UBIFS_1 ";"                                   \
    "setenv fsstandby " UBIFS_0 ";"                                  \
    "ubi part " UBI_PART ";"                                         \
    "ubifsmount " UBI_VOLUME ":$fsactive;"                           \
    "if ubifsload $loadaddr primary.flag; then"                      \
    " setenv fsactive " UBIFS_0 ";"                                  \
    " setenv fsstandby " UBIFS_1 ";"                                 \
    " ubifsmount " UBI_VOLUME ":$fsactive;"                          \
    " echo \"Fallback boot from $fsactive\";"                        \
    "else"                                                           \
    " echo \"Fallback boot from $fsactive\";"                        \
    "fi;"                                                            \
    "if ubifsload $loadaddr " FIT_IMAGE "; then"                     \
    " setenv bootbankargs fsactive=$fsactive fsstandby=$fsstandby;"  \
    " run secureboot;"                                               \
    "fi;"                                                            \
    "reset;"

#define BOOT_FAILED_HANG_COMMAND      \
    "ledc all off;"                   \
    "while true ; do"                 \
    "  ledc all red;"                 \
    "  sleep 1;"                      \
    "  ledc all off;"                 \
    "  ledc pwr green;"               \
    "  sleep 1;"                      \
    "done"

#define CONFIG_EXTRA_ENV_SETTINGS                                            \
    "bootlimit=" __stringify(CONFIG_BOOTCOUNT_LIMIT) "\0"                    \
    "consoledev=ttyS0\0"                                                     \
    "loadaddr=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0"                       \
    "othbootargs=" __stringify (OTHERBOOTARGS) "\0"                          \
    "mtdids=" MTDIDS_DEFAULT "\0"                                            \
    "mtdparts=" MTDPARTS_DEFAULT "\0"                                        \
    "secureboot=" SECURE_BOOT_COMMAND "\0"                                   \
    "bootfailedhang=" BOOT_FAILED_HANG_COMMAND "\0"                          \
    "ubiboot=" UBIBOOT_COMMAND "\0"                                          \
    "ubialtboot=" UBIALTBOOT_COMMAND "\0"                                    \
    "altbootcmd=run ubialtboot;\0"                                           \
    "netdev=eth0\0"

#define CONFIG_BOOTCOMMAND  "run ubiboot"
    
   
#endif /* __CONFIG_H__ */
