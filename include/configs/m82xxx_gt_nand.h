/*
 * Copyright(c) 2007-2015 Intel Corporation. All rights reserved.
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
#ifndef __CONFIG_H
#define __CONFIG_H

#include <asm/arch/hardware.h>

#include "mindspeed_version.h"

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_GT_BOARD
#define CONFIG_ADT75
#define CONFIG_TURNER
#define CONFIG_GTH
#define CONFIG_T2K
//#define CONFIG_DUAL_CORE

#define CONFIG_SYS_MEMTEST_START    0x0
#define CONFIG_SYS_MEMTEST_END      0x10000
#define CFG_HZ                     (1000)
#define CONFIG_SYS_HZ              CFG_HZ
#define CONFIG_SYS_HZ_CLOCK        (ARMCLK_HZ/2)   /* Timers clocked AXI clock at 1MHz*/
#define CONFIG_SYS_TIMER_RELOAD    0xFFFFFFFF
#define TIMER_LOAD_VAL             CONFIG_SYS_TIMER_RELOAD

#define CONFIG_CMDLINE_EDITING
#define CONFIG_AUTO_COMPLETE
#define CONFIG_CMDLINE_TAG      1   /* enable passing of ATAGs  */
#define CONFIG_SETUP_MEMORY_TAGS    1
#define CONFIG_MISC_INIT_R      1   /* call misc_init_r during start up */

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN   (CONFIG_ENV_SIZE + 128*1024)
#define CONFIG_SYS_GBL_DATA_SIZE    128 /* size in bytes reserved for initial data */

#define CONFIG_COMCERTO_GEMAC
#define CONFIG_COMCERTO_MII_CFG_BOOTSTRAP

//TODO fixme, trying GEM1 addres according to doc (GEM0 for model)
#ifdef CONFIG_RTSM_ONLY
#define GEMAC_BASEADDR         GEMAC1_BASEADDR
#else
#define GEMAC_BASEADDR         GEMAC0_BASEADDR
#endif

// SGMII loopback
//#define SGMII_AUTONEG_MODE

#define GEMAC0_PHY_ADDR         0
#define GEMAC0_CONFIG           CONFIG_COMCERTO_USE_SGMII
#define GEMAC0_MODE             (GEMAC_GEM_MODE | GEMAC_SW_CONF | GEMAC_SW_FULL_DUPLEX | GEMAC_SW_SPEED_1G | GEMAC_GEM_MODE_SGMII)
#define GEMAC0_PHY_FLAGS        GEMAC_PHY_AUTONEG
#define GEMAC0_PHYIDX           0

#define GEMAC1_PHY_ADDR         6
#define GEMAC1_CONFIG           CONFIG_COMCERTO_USE_RGMII
#define GEMAC1_MODE             (GEMAC_GEM_MODE | GEMAC_SW_CONF | GEMAC_SW_FULL_DUPLEX | GEMAC_SW_SPEED_1G | GEMAC_GEM_MODE_RGMII)
#define GEMAC1_PHY_FLAGS        GEMAC_PHY_AUTONEG
#define GEMAC1_PHYIDX           0

#define CONFIG_MII              /* MII PHY management       */
#define CONFIG_PHY_GIGE         /* MII PHY supports gigabit ethernet */
/*
 * UART 16550 Configuration
 */
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE     4
#define CONFIG_SYS_NS16550_CLK          (1000000*get_bus_clock())
#define CONFIG_SYS_NS16550_COM1         UART0_BASE
#define CONFIG_CONS_INDEX               1

#define CONFIG_SYS_NS16550_COM2         UART1_BASEADDR
// #define CONFIG_CONS_INDEX        2

#define CONFIG_BAUDRATE         115200
#define CONFIG_SYS_BAUDRATE_TABLE   { 9600, 19200, 38400, 57600, 115200 }

/*
 * Command line configuration.
 */
#define CONFIG_CMD_BDI
#define CONFIG_CMD_ELF
#define CONFIG_CMD_ENV
#define CONFIG_CMD_IMI
#define CONFIG_CMD_MEMORY
#define CONFIG_CMD_SAVEENV
#define CONFIG_CMD_NET
#define CONFIG_NET_MULTI
#define CONFIG_CMD_PING
#define CONFIG_CMD_I2C
#define CONFIG_CMD_NAND
#define CONFIG_NET_RETRY_COUNT  50
#define CONFIG_CMD_RUN
// #define CONFIG_CMD_NORML
#define CONFIG_CMD_SYSINFO

// #define CONFIG_CMD_SPI
// #define CONFIG_CMD_DEJITTER
// #define CONFIG_CMD_SFP
#define CONFIG_CMD_MII

/*
 * Memory setting (512M per DDR controller)
 */
#define DDR0_BASE       0x00000000
#define DDR0_SIZE       0x10000000
//#define DDR_2_RANKS

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_MASK   CONFIG_BOOTP_DEFAULT

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP /* undef to save memory      */
#define CONFIG_SYS_CBSIZE   1024        /* Console I/O Buffer Size  */
/* Monitor Command Prompt   */
#define CONFIG_IDENT_STRING     MINDSPEED_VERSION "\n\nIntel m82xxx nand loader\n"
#define CONFIG_SYS_MAXARGS  512     /* max number of command args    */
#define CONFIG_SYS_BARGSIZE CONFIG_SYS_CBSIZE   /* Boot Argument Buffer Size        */

#undef  CONFIG_SYS_CLKS_IN_HZ       /* everything, incl board info, in Hz */
#define CONFIG_SYS_LOAD_ADDR    0x7FC0  /* default load address */

/*-----------------------------------------------------------------------
 * Stack sizes
 *
 * The stack sizes are set up in start.S using the settings below
 */
#define CONFIG_STACKSIZE    (128*1024)  /* regular stack */

/*-----------------------------------------------------------------------
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS    1       /* we have 1 bank of DRAM */
#define PHYS_SDRAM_1        0x00000000  /* SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE   0x80000000  /* 2 GB for Linux */
#define PHYS_FLASH_SIZE         (0x0080000) /* 512 K EEPROM */
#define CONFIG_SYS_PROMPT   "T2000# "
/*
 * Note that the following are example environment settings.
 * They will need to be changed  to settings for the local network,
 * using the u-boot command line setenv functionality
 */
//#define CONFIG_BOOTARGS "root=ubi0:rootfs ubi.mtd=0,2048 rootfstype=ubifs rw mem=512M cram_offset=0xa000 icc_heap_size=2M icc_part_size=86M earlycon=uart8250,mmio32,0xfe800000,115200n8 console=ttyS0
#define CONFIG_BOOTARGS "root=/dev/mtdblock1 rootfstype=jffs2 rw mem=512M cram_offset=0x23000 icc_heap_size=2M icc_part_size=86M earlycon=uart8250,mmio32,0xfe800000,115200n8 console=ttyS0 ip=10.1.178.238::10.1.178.1:255.255.0.0::eth2:off noswap nopcie hwaddress=eth2,00:15:17:14:C6:00"
#define CONFIG_BOOTCOMMAND "run bootaa"
#define CONFIG_BOOTDELAY 3

/*
 * Static configuration when assigning fixed address
 */
#define CONFIG_NETMASK  255.255.255.0   /* talk on MY local net */
#define CONFIG_IPADDR   192.168.1.2 /* static IP I currently own */
#define CONFIG_SERVERIP 192.168.1.1 /* current IP of my dev pc */
//#define   CONFIG_EXTRA_ENV_SETTINGS "ethaddr=00:15:17:14:C6:A3\0"
// 00:15:17:14:C5:xx - for T2k checkout
// 00:15:17:14:C6:xx - for T2k EVM
// 00:15:17:14:C7:xx - for T3300
// xx - last 2 digits of board number
#define CONFIG_ETHADDR 00:15:17:14:C6:00
#define CONFIG_OVERWRITE_ETHADDR_ONCE
#define CONFIG_EXTRA_ENV_SETTINGS               \
    "ethprime=gemac1\0"                     \
    "eth1addr=00:15:17:14:C6:01\0" \
    "cspname=kernel.uImage\0" \
    "tftpdir=.\0" \
    "updateboot=tftp ${serverip}:${tftpdir}/u-boot.bin;nand erase 0 40000;nand write ${fileaddr} 0 40000\0" \
    "updatecsp=tftp ${serverip}:${tftpdir}/${cspname};nand erase 80000 1180000;nand write ${fileaddr} 80000 1180000\0" \
    "updatefs=tftp ${serverip}:${tftpdir}/${fsname};nand erase 1200000 ee00000;nand write ${fileaddr} 1200000 $(filesize)\0" \
    "bootaa=nand read 1000000 80000 1180000; bootm 1000000\0" \
    ""

/*
 * I2C support
 */
#if defined(CONFIG_CMD_I2C)
#define CONFIG_HARD_I2C             1        /* Use I2C controller, not GPIO bit bang */
#define CONFIG_SYS_I2C_SPEED                    100000   /* 100 kHz speed */
#define CONFIG_SYS_I2C_SLAVE                    0x7F     /* Slave address 0x7F */
#define CONFIG_SYS_I2C_EEPROM_ADDR              0x50     /* I2C device address of I2C EEPROM */
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN          2        /* Bytes of address, 2 bytes for 64 kilobyte EEPROM */
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS       6 /* max 64 byte */
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS   10       /* Wait 10 milliseconds after page write operation */
#define CONFIG_SYS_EEPROM_PAGE_WRITE_ENABLE 1        /* Page write enabled */
#define CONFIG_SYS_EEPROM_SIZE              (64*1024)/* I2C EEPROM total size of 64 kilobytes */
#endif

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */

/*
 * NAND device support options
 */
#if defined(CONFIG_CMD_NAND)
#define CONFIG_SYS_MAX_NAND_DEVICE  1
#define CONFIG_SYS_NAND_BASE        0xAC000000
#endif

#define CONFIG_ENV_IS_IN_NAND       1
//#define CONFIG_ENV_IS_IN_SPI  1
#define CONFIG_ENV_OFFSET       0x040000
#define CONFIG_ENV_SIZE         0x020000

/* no NOR flash */
#define CONFIG_SYS_NO_FLASH                     1


/*
 * Since we don't know which end has the small erase blocks
 * or indeed whether they are present
 * we use the penultimate full sector location
 * for the environment - we save a full sector even tho
 * the real env size CONFIG_ENV_SIZE is probably less
 * Maintain the distinction since we want to make stack size as small as possible
 */

#define CONFIG_SYS_SERIAL0  (serial_base + 0)
#define CONFIG_SYS_SERIAL1  (serial_base + 0x00001000)

/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE   (CONFIG_SYS_CBSIZE+strlen(gd->bd->prompt)+16)

#endif

