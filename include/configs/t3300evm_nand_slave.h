#ifndef __CONFIG_T3300EVM_NAND_H                                                                                    
#define __CONFIG_T3300EVM_NAND_H

#include <linux/sizes.h>

#include "m82xxx_asic_nor16.h"

#if defined(DEBUG)
#undef DEBUG
#endif

#undef CONFIG_ENV_IS_IN_FLASH
#undef CONFIG_ENV_OFFSET
#undef CONFIG_ENV_SIZE
#undef CONFIG_SYS_MALLOC_LEN
#undef CONFIG_SYS_LOAD_ADDR
#undef CONFIG_LOADADDR
#undef CONFIG_BOOTCOMMAND
#undef CONFIG_BOOTARGS


#define CONFIG_SYS_MALLOC_LEN   (CONFIG_ENV_SIZE + 1024*1024)
#define CONFIG_SYS_LOAD_ADDR	0x02000000
#define CONFIG_LOADADDR		0x02000000

#ifndef CONFIG_CMDLINE_TAG
#define CONFIG_CMDLINE_TAG 1
#endif
#ifndef CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_SETUP_MEMORY_TAGS 1
#endif
#ifndef CONFIG_INITRD_TAG
#define CONFIG_INITRD_TAG 1
#endif

#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_CMD_ECHO

#define CONFIG_ENV_SIZE         8192
#define CONFIG_ENV_OVERWRITE
#define CONFIG_ENV_IS_IN_EEPROM
#define CONFIG_ENV_OFFSET 0


#define CONFIG_RBTREE

#define CONFIG_LZO

#define CONFIG_PCIE
#define CONFIG_TURNER
#define CONFIG_CMD_PCIE
#define CONFIG_CMD_T3300

#define CONFIG_FIT

#define CONFIG_ETHPRIME		"gemac1"
#define LINUX_CONSOLEDEV	"ttyS0"
#define CMD_LINE_ARGS_LINUX									\
	"console=" LINUX_CONSOLEDEV "," __stringify(CONFIG_BAUDRATE) "n8"			\
	" elevator=noop "									\
	" mem=256M"										\
	" hwaddress=eth1,${ethaddr},eth2,${eth1addr}"						\
	" icc_heap_size=2M icc_part_size=384M ddr_limit=2G cram_offset=0x24000"			\
	" ipa_oui=${ipa_oui}"									\
	" ipa_serial=${ipa_serial}"								\
	" ipa_hwchar=${ipa_hwchar}"								\
	" ipa_pai=${ipa_pai}"									\
	" ipa_secmode=${ipa_secmode}"								\
	" ipa_loader_revocation=${ipa_loader_revocation}"					\
	" ipa_app_revocation=${ipa_app_revocation}"

#define SECURE_BOOT_COMMAND									\
	"bootm ${loadaddr}#${selected_config}; "

/*
 * Something like the following is required to boot this board:
 *
 * env default -a
 *
 * setenv ethaddr 00:15:E1:18:b4:90
 * setenv eth1addr 00:15:E1:18:b4:91
 * setenv ipa_oui 0015E1
 * setenv ipa_serial 0123456789
 * setenv ipa_hwchar 0x38
 * setenv ipa_pai 0x4141
 * setenv ipa_secmode d
 * setenv ipa_loader_revocation 0
 * setenv ipa_app_revocation 0
 *
 * env save
 * env save
 */

/*TODO get fitimage over pcie to expected address*/
#define IPABOOT_COMMAND										\
	"if test -n \"${ipa_oui}\" -a -n \"${ipa_serial}\" -a -n \"${ipa_hwchar}\" -a "		\
		"-n \"${ipa_pai}\" -a -n \"${ipa_secmode}\" -a "				\
		"-n \"${ipa_loader_revocation}\" -a -n \"${ipa_app_revocation}\" -a "		\
		"-n \"${ethaddr}\" -a -n \"${eth1addr}\"; then "				\
	"  run select_bootargs; "								\
	"  run select_config; "									\
	"  run secureboot;"									\
	"else "											\
	"  echo \"This board has not been characterised yet.  Please set up all required "	\
		"environment variables, save the environment and reboot.\"; "			\
	"fi; "											\
	"echo \"ERROR: Failed to load and run a kernel on this board, doh!\"; "

#define SET_BOOTARGS										\
	"setenv bootargs " CMD_LINE_ARGS_LINUX

#define SELECT_CONFIG										\
	"setenv selected_config config@1; "

#define CONFIG_EXTRA_ENV_SETTINGS								\
	"autoload=no\0"										\
	"autostart=no\0"									\
	"ethact=" CONFIG_ETHPRIME "\0"								\
	"ethprime=" CONFIG_ETHPRIME "\0"							\
	"ethrotate=no\0"									\
	"netdev=eth1\0"										\
	"select_bootargs=" SET_BOOTARGS "\0"							\
	"select_config=" SELECT_CONFIG "\0"							\
	"ipaboot=" IPABOOT_COMMAND "\0"								\
	"secureboot=" SECURE_BOOT_COMMAND "\0"

#define CONFIG_BOOTCOMMAND									\
	"run ipaboot"

#define CONFIG_SYS_NO_FLASH
#undef CONFIG_MTD_PARTITIONS
#undef CONFIG_MTD_CONCAT
#undef CONFIG_HAS_DATAFLASH
#undef CONFIG_FLASH_CFI_DRIVER
#undef CONFIG_FLASH_CFI_MTD 
#undef CONFIG_HAS_DATAFLASH
#undef CONFIG_FTSMC020
#undef CONFIG_FLASH_CFI_LEGACY

#endif
