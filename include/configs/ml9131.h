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

#define CONFIG_DISABLE_IMAGE_LEGACY
#define CONFIG_ENV_IS_NOWHERE
#define CONFIG_ENV_ADDR        (CONFIG_SYS_MONITOR_BASE - 0x1000)
#define CONFIG_ENV_SIZE        0x2000 /* 8k for the environment - should be enough */

#define CONFIG_ETHPRIME "eTSEC2"

#define CONFIG_EXTRA_ENV_SETTINGS                                              \
	"netdev=eth1\0"                                                         \
	"ethrotate=no\0"

#define STANDARD_BOOT_COMMAND							\
	"if nand read 200000 0x200000 0x100000; then "				\
	 "go 200000; "								\
	"fi; "									\
	"reset"

#define CONFIG_BOOTCOMMAND STANDARD_BOOT_COMMAND
#define CONFIG_BOOTDELAY 2

#define CONFIG_CMD_FIMAGE

/*TODO remove the following config options, they are only here for debugging during development.*/
#if 1
#define CONFIG_MD5
#define CONFIG_CMD_MD5SUM
#endif

#endif
