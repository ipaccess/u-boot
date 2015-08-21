#ifndef __CONFIG_H
#define __CONFIG_H

//#include <asm/arch/hardware.h>
#include <asm/hardware.h>

#include "mindspeed_version.h"

#define CONFIG_SYS_TEXT_BASE 0x20000000  //For testing
//#define CONFIG_SYS_TEXT_BASE 0x0C800000 //image in NOR flash

#define CONFIG_SYS_INIT_SP_ADDR (IRAM_BASE + 0x800)

// #define CONFIG_RTSM_ONLY

#define DEBUG 1
// #define CONFIG_MTD_DEBUG
// #define CONFIG_MTD_DEBUG_VERBOSE	100

// #define CONFIG_PCIE_INIT
#define CONFIG_PCIE

// Additional serdes init for build VSEMI
// #define TRANSCEDE_SERDES_INIT

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_TURNER
#define CONFIG_GTH
#define CONFIG_T2K
//#define CONFIG_DUAL_CORE

#define CONFIG_SYS_MEMTEST_START	0x0
#define CONFIG_SYS_MEMTEST_END		0x10000
#define CFG_HZ				(1000)
#define CONFIG_SYS_HZ			CFG_HZ
#define CONFIG_SYS_HZ_CLOCK		(ARMCLK_HZ/2)	/* Timers clocked AXI clock at 1MHz*/
#define CONFIG_SYS_TIMER_RELOAD		0xFFFFFFFF
#define TIMER_LOAD_VAL			CONFIG_SYS_TIMER_RELOAD

#define CONFIG_CMDLINE_TAG		1	/* enable passing of ATAGs	*/
#define CONFIG_SETUP_MEMORY_TAGS	1
#define CONFIG_MISC_INIT_R		1	/* call misc_init_r during start up */

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN	(CONFIG_ENV_SIZE + 128*1024)
#define CONFIG_SYS_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */

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

#define GEMAC1_PHY_ADDR         5
#define GEMAC1_CONFIG		CONFIG_COMCERTO_USE_RGMII
#define GEMAC1_MODE		(GEMAC_GEM_MODE | GEMAC_SW_CONF | GEMAC_SW_FULL_DUPLEX | GEMAC_SW_SPEED_1G | GEMAC_GEM_MODE_RGMII)
#define GEMAC1_PHY_FLAGS        GEMAC_PHY_AUTONEG
#define GEMAC1_PHYIDX           0

#define CONFIG_MII              /* MII PHY management		*/
#define CONFIG_BITBANGMII       /* bit-bang MII PHY management	*/
#define CONFIG_PHY_GIGE         /* MII PHY supports gigabit ethernet */

/*
 * GPIO defines used for bit-banged MII communications
 */
#define MDIO_PORT	2		/* Number assigned to bit bang MDIO port */

/** @brief mapping for MDIO Data (MDIO) GPIO for T2200 and T3300 EVM boards,
 * also mapped to debug LED 1 (0 for on) on T2200 EVM board
 */
#define MDIO_GPIO 20

#define MDIO_GPIO_SELECT_MASK (1 << ((MDIO_GPIO - 16) * 2))
#define MDIO_GPIO_SELECT_REG  GPIO_31_16_PIN_SELECT_REG

/** @brief mapping for MDIO Clock (MDC) GPIO for T2200 & T3300 EVM boards */
#define MDC_GPIO  21
#define MDC_GPIO_SELECT_MASK (1 << ((MDC_GPIO - 16) * 2))
#define MDC_GPIO_SELECT_REG  GPIO_31_16_PIN_SELECT_REG

#define MDIO_GPIO_BIT   (1<<MDIO_GPIO)
#define MDC_GPIO_BIT    (1<<MDC_GPIO)

#define MDIO_ACTIVE	REG32(GPIO_OE_REG) |=   MDIO_GPIO_BIT
#define MDIO_TRISTATE   REG32(GPIO_OE_REG) &= ~(MDIO_GPIO_BIT)

#define MDIO_READ	((REG32(GPIO_INPUT_REG) & MDIO_GPIO_BIT) != 0)

#define MDIO_SET        REG32(GPIO_OUTPUT_REG) |=   MDIO_GPIO_BIT
#define MDIO_CLR        REG32(GPIO_OUTPUT_REG) &= ~(MDIO_GPIO_BIT)

#define MDC_SET         REG32(GPIO_OUTPUT_REG) |=   MDC_GPIO_BIT
#define MDC_CLR         REG32(GPIO_OUTPUT_REG) &= ~(MDC_GPIO_BIT)

#define MDIO(bit)	if(bit) MDIO_SET; \
			else	MDIO_CLR

#define MDC(bit)	if(bit) MDC_SET; \
			else	MDC_CLR

#define MIIDELAY	udelay(1)

/*
 * UART 16550 Configuration
 */
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
//#define CONFIG_SYS_NS16550_REG_SIZE     4
#define CONFIG_SYS_NS16550_REG_SIZE     -4
#define CONFIG_SYS_NS16550_CLK 		    (1000000*get_bus_clock())
#define CONFIG_SYS_NS16550_COM1 	    UART0_BASE
#define CONFIG_CONS_INDEX 		        1

#define CONFIG_SYS_NS16550_COM2 	    UART1_BASEADDR
// #define CONFIG_CONS_INDEX 		2

#define CONFIG_BAUDRATE			115200
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

/*
 * Command line configuration.
 */
#define CONFIG_CMD_BDI
#define CONFIG_CMD_ELF
#define CONFIG_CMD_ENV
#define CONFIG_CMD_FLASH
#define CONFIG_CMD_IMI
#define CONFIG_CMD_MEMORY
#define CONFIG_CMD_SAVEENV
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_NET
#define CONFIG_CMD_NET_ETHDIAG
#define CONFIG_NET_MULTI
#define CONFIG_CMD_PING
#define CONFIG_CMD_I2C
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_NAND
#define CONFIG_NET_RETRY_COUNT	50
#define CONFIG_CMD_PCIE
#define CONFIG_CMD_T3300
#define CONFIG_CMD_RUN
#define CONFIG_CMD_LOADB
// #define CONFIG_CMD_NORML
#define CONFIG_CMD_UPGRADE
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
#define CONFIG_SYS_SDRAM_BASE DDR0_BASE
//#define DDR_2_RANKS

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_MASK	CONFIG_BOOTP_DEFAULT

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP	/* undef to save memory		 */
#define CONFIG_SYS_CBSIZE	1024		/* Console I/O Buffer Size	*/
/* Monitor Command Prompt   */
#define CONFIG_IDENT_STRING 	MINDSPEED_VERSION "\n\nIntel m82xxx nor16 loader\n"
#define CONFIG_SYS_MAXARGS	512		/* max number of command args	 */
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size		*/

#undef	CONFIG_SYS_CLKS_IN_HZ		/* everything, incl board info, in Hz */
#define CONFIG_SYS_LOAD_ADDR	0x7FC0	/* default load address */

/*-----------------------------------------------------------------------
 * Stack sizes
 *
 * The stack sizes are set up in start.S using the settings below
 */
#define CONFIG_STACKSIZE	(128*1024)	/* regular stack */

/*-----------------------------------------------------------------------
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS	1		/* we have 1 bank of DRAM */
#define PHYS_SDRAM_1		0x00000000	/* SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE	0x80000000	/* 2 GB for Linux */
#define PHYS_FLASH_SIZE         (0x0080000)	/* 512 K EEPROM */
#define CONFIG_SYS_PROMPT	"T2000# "
/* 
 * Note that the following are example environment settings.
 * They will need to be changed  to settings for the local network, 
 * using the u-boot command line setenv functionality
 */
#define CONFIG_BOOTARGS "root=/dev/nfs rw nfsroot=10.1.35.172:/opt/openwrt-2.1.0-rc0 mem=256M earlycon=uart8250,mmio32,0xfe800000,115200n8 console=ttyS0 ip=dhcp noswap nopcie"
#define CONFIG_BOOTCOMMAND "setenv bootargs ${bootargs} hwaddress=eth1,${ethaddr}; dhcp 10.1.35.172:t2200.uImage; bootm"
#define CONFIG_BOOTDELAY 3

/*
 * Static configuration when assigning fixed address
 */
#define CONFIG_NETMASK	255.255.255.0	/* talk on MY local net */
#define CONFIG_IPADDR	192.168.1.2	/* static IP I currently own */
#define CONFIG_SERVERIP	192.168.1.1	/* current IP of my dev pc */
//#define	CONFIG_EXTRA_ENV_SETTINGS "ethaddr=00:15:17:14:C6:A3\0"
// 00:15:17:14:C5:xx - for T2k checkout
// 00:15:17:14:C6:xx - for T2k EVM
// 00:15:17:14:C7:xx - for T3300
// xx - last 2 digits of board number
#define CONFIG_ETHADDR 00:15:17:14:C6:00
#define CONFIG_OVERWRITE_ETHADDR_ONCE
#define CONFIG_BOOTFILE	"kernel-c4k-rtsm-0.1.5.uImage" /* file to load */

/*
 * SPI driver
 */
#define CFG_SPI_CLOCK_POLARITY          1               /* Inactive state of serial clock is high */
#define CFG_SPI_CLOCK_PHASE             1               /* Serial Clock toggles at the start of the first data bit */
#define CFG_SPI_FRAMESIZE               8               /* define the SPI frame size in bits. 8-bit supported only */
#define CFG_SPI_CLOCK_SPEED             2000000         /* the SPI speed in Hz */


/*
 * I2C support
 */
#if defined(CONFIG_CMD_I2C)
#define CONFIG_HARD_I2C				1        /* Use I2C controller, not GPIO bit bang */
#define CONFIG_SYS_I2C_SPEED                  	100000   /* 100 kHz speed */
#define CONFIG_SYS_I2C_SLAVE                  	0x7F     /* Slave address 0x7F */
#define CONFIG_SYS_I2C_EEPROM_ADDR            	0x50     /* I2C device address of I2C EEPROM */
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN        	2        /* Bytes of address, 2 bytes for 64 kilobyte EEPROM */
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS     	6        /* max 64 byte */
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS 	10       /* Wait 10 milliseconds after page write operation */
#define CONFIG_SYS_EEPROM_PAGE_WRITE_ENABLE	1        /* Page write enabled */
#define CONFIG_SYS_EEPROM_SIZE		      	(64*1024)/* I2C EEPROM total size of 64 kilobytes */
#endif

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */

/*
 * NAND device support options
 */
#if defined(CONFIG_CMD_NAND) 
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		0xAC000000
#endif

/*
 *  Use the CFI flash driver for ease of use
 */
#define CONFIG_SYS_FLASH_CFI
#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_SYS_FLASH_CFI_WIDTH	2
#undef CONFIG_FLASH_CFI_LEGACY
#undef CONFIG_SYS_FLASH_LEGACY_512Kx8
#define CONFIG_ENV_IS_IN_FLASH		1		/* env in flash */
#define CONFIG_SYS_FLASH_BASE		0xA0000000
#define CONFIG_SYS_MAX_FLASH_BANKS	(1)		/* max number of memory banks */

/* timeout values are in ticks */
#define CONFIG_SYS_FLASH_ERASE_TOUT	(200*CFG_HZ)	/* Timeout for Flash Erase */
#define CONFIG_SYS_FLASH_WRITE_TOUT	(200*CFG_HZ)	/* Timeout for Flash Write */

#define CONFIG_SYS_MAX_FLASH_SECT	(4096)		/* The maximum number of sectors we may need to hold data about */
#define FLASH_MAX_SECTOR_SIZE	(0x00040000)		/* 256 KB sectors */
#define FLASH_MIN_SECTOR_SIZE	(0x00020000)		/* 128 KB sectors */
#define CONFIG_SYS_FLASH_BUFFER_WRITE	1
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE	1

#define CONFIG_FLASH_SHOW_PROGRESS	1
/* Room required on the stack for the environment data */
#define CONFIG_ENV_SIZE         8192

/* 
 * Since we don't know which end has the small erase blocks 
 * or indeed whether they are present
 * we use the penultimate full sector location
 * for the environment - we save a full sector even tho 
 * the real env size CONFIG_ENV_SIZE is probably less
 * Maintain the distinction since we want to make stack size as small as possible 
 */

/* Amount of flash used for environment */
#define CONFIG_ENV_SECT_SIZE	FLASH_MIN_SECTOR_SIZE 

/* 
 * Top of flash must NOT be defined using sectors
 * Board variants may have differing flash components
 */ 
#define FLASH_TOP		(CONFIG_SYS_FLASH_BASE + PHYS_FLASH_SIZE)

#define CONFIG_ENV_ADDR		(FLASH_TOP - (CONFIG_ENV_SECT_SIZE))
#define CONFIG_ENV_OFFSET	(CONFIG_ENV_ADDR - CONFIG_SYS_FLASH_BASE)

#define CONFIG_SYS_SERIAL0	(serial_base + 0)
#define CONFIG_SYS_SERIAL1	(serial_base + 0x00001000) 

/* Print Buffer Size */
//#define CONFIG_SYS_PBSIZE 	(CONFIG_SYS_CBSIZE+strlen(gd->bd->prompt)+16)
#define CONFIG_SYS_PBSIZE 	(CONFIG_SYS_CBSIZE+16)

#define CONFIG_USE_PRIVATE_LIBGCC

#endif

