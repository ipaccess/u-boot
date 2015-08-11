#ifndef __ML9131_CONFIG_H
#define __ML9131_CONFIG_H

#define CONFIG_ML9131
#define CONFIG_IPA9131
#define CONFIG_BSC9131RDB
#define CONFIG_NAND
#define CONFIG_SYS_CLK_100

#include "BSC9131RDB.h"

#undef CONFIG_OF_LIBFDT
#undef CONFIG_OF_BOARD_SETUP
#undef CONFIG_OF_STDOUT_VIA_ALIAS
#undef CONFIG_FIT
#undef CONFIG_FIT_VERBOSE
#undef CONFIG_ENV_IS_IN_NAND
#undef CONFIG_SYS_EXTRA_ENV_RELOC
#undef CONFIG_ENV_SIZE
#undef CONFIG_ENV_OFFSET
#undef CONFIG_ENV_RANGE
#undef CONFIG_HOSTNAME
#undef CONFIG_ROOTPATH
#undef CONFIG_BOOTFILE
#undef CONFIG_UBOOTPATH
#undef CONFIG_EXTRA_ENV_SETTINGS
#undef CONFIG_RAMBOOTCOMMAND
#undef CONFIG_BOOTCOMMAND
#undef CONFIG_ETHPRIME

#undef TSEC2_PHY_ADDR
#define TSEC2_PHY_ADDR		1

/* The microloader uses the Freescale SEC engine */
#define CONFIG_FSL_CAAM

/* disable the thermal alert for now - this needs board mods */
#define CONFIG_ML9131_NO_THERMAL_ALERT

#define CONFIG_TFM
#define CONFIG_SHA1
#define CONFIG_SHA256
#define CONFIG_DISABLE_IMAGE_LEGACY
#define CONFIG_ENV_IS_NOWHERE
#define CONFIG_ENV_ADDR        (CONFIG_SYS_MONITOR_BASE - 0x1000)
#define CONFIG_ENV_SIZE        0x2000 /* 8k for the environment - should be enough */
#define CONFIG_MBED_TLS

#define CONFIG_CHARACTERISATION_IPA9131
#define CONFIG_CHARACTERISATION_EEPROM_ADDR SPD_EEPROM_ADDRESS
#define CONFIG_CHARACTERISATION_IPA9131_OFFSET 0
#define CONFIG_CHARACTERISATION_IPA9131_SIZE 256
#define CONFIG_CHARACTERISATION_IPA9131_VERSION 0x0
#define CONFIG_CMD_CHARACTERISE_HW

#define CONFIG_MISC_INIT_R
#define CONFIG_CMD_NAND_TRIMFFS
#define CONFIG_MTD_DEVICE
#define CONFIG_MTD_PARTITIONS
#define CONFIG_CMD_MTDPARTS
#define MTD_PARTITION_DEFAULT   "nand0,0"
#define MTDIDS_DEFAULT          "nand0=ff800000.flash"
#define MTDPARTS_DEFAULT                \
        "mtdparts=ff800000.flash:"      \
        "1024k@0k(ML),"                 \
        "1024k@1024k(UBOOT0),"          \
        "1024k@2048k(UBOOT1),"          \
        "128k@3328k(BLOB0),"            \
        "127488k@3456k(FS),"            \
        "128k@130944k(BLOB1)"

/* Auto-boot options */
#define CONFIG_BOOTDELAY 3
#define CONFIG_AUTOBOOT_KEYED
#define CONFIG_AUTOBOOT_STOP_STR "stop"
#define CONFIG_AUTOBOOT_PROMPT "ml9131: autoboot in %d seconds (\"stop\" to stop)\n",bootdelay

/* Monitor Command Prompt */
#define CONFIG_SYS_PROMPT   "ml9131=> "

/*
 * IPA Commands
 */
#define CONFIG_CMD_LEDC

#define CONFIG_ETHPRIME "eTSEC2"

#define CONFIG_EXTRA_ENV_SETTINGS	\
	"netdev=eth1\0"			\
	"ethrotate=no\0"

#define STANDARD_BOOT_COMMAND			\
	"ledc all yellow off 1 300; "	 	\
	"restore_raw_container -m 3 -m 5; "	\
	"if test $? -eq 1; then "		\
 	 "ledc all green red 3 1000; "		\
	 "reset; "				\
	"fi; "					\
	"gen_trusted_desc 0x36F00000; "		\
	"ml9131; "				\
	"ledc nwk green red 3 1000; "		\
	"reset"

#define CONFIG_BOOTCOMMAND STANDARD_BOOT_COMMAND

#define CONFIG_CMD_SEC_GEN_TRUSTED_DESC
/* temporary, to see Sec RNG generates random number */
#define CONFIG_CMD_SEC_GEN_RN

#endif
