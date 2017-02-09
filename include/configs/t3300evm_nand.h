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

#define NAND_FLASH_SECTOR_SIZE      (SZ_128K)
#define CONFIG_ENV_IS_IN_NAND
#define CONFIG_ENV_OFFSET           (NAND_FLASH_SECTOR_SIZE * 8)
#define CONFIG_ENV_SIZE         8192
#define CONFIG_ENV_OFFSET_REDUND    (NAND_FLASH_SECTOR_SIZE * 18)
#define CONFIG_ENV_OVERWRITE

#ifndef CONFIG_CMD_NAND
#define CONFIG_CMD_NAND
#endif
#define CONFIG_CMD_NAND_TRIMFFS

#define CONFIG_MTD_DEVICE
#define CONFIG_CMD_MTDPARTS
#define CONFIG_MTD_PARTITIONS
#define CONFIG_FLASH_CFI_MTD

#define MTD_PARTITION_DEFAULT	"nand0,0"
#define MTDIDS_DEFAULT		"nand0=gen_nand,nor0=physmap-flash.0"

#define MTDPARTS_DEFAULT		        \
	"mtdparts="			        \
	    "gen_nand:"		                \
		"1024K@0K(ML0),"		\
		"128K@1152K(RAW0),"		\
		"1024K@1280K(ML1),"		\
		"128K@2432K(RAW1),"		\
		"1024K@2560K(UBOOT0),"		\
		"128K@3712K(RAW2),"		\
		"1024K@3840K(UBOOT1),"		\
		"128K@4992K(RAW3),"		\
		"-@5120K(FS);"			\
	    "physmap-flash.0:"		        \
		"256K@0K(MML),"			\
		"1024K@256K(MUBOOT),"		\
		"128K@1280K(MRAW0),"		\
		"256K@1408K(SML),"		\
		"1024K@1664K(SUBOOT),"		\
		"128K@1792K(SRAW0),"		\
		"-@1920K(RES)"

	

#define CONFIG_CMD_UBI
// #define CONFIG_UBI_SILENCE_MSG
#define CONFIG_RBTREE

#define CONFIG_CMD_UBIFS
#define CONFIG_LZO

#define CONFIG_PCIE
#define CONFIG_TURNER
#define CONFIG_CMD_PCIE
#define CONFIG_CMD_T3300

#define PCIE_INITIALIZED_IN_ML

// #define CONFIG_UBIFS_SILENCE_MSG

#define CONFIG_FIT

#define CONFIG_ETHPRIME		"gemac1"
#define LINUX_CONSOLEDEV	"ttyS0"
#define CMD_LINE_ARGS_LINUX									\
	"console=" LINUX_CONSOLEDEV "," __stringify(CONFIG_BAUDRATE) "n8"			\
	" elevator=noop "									\
	MTDPARTS_DEFAULT									\
	" mem=512M"										\
	" hwaddress=eth1,${ethaddr},eth2,${eth1addr}"						\
	" icc_heap_size=132M icc_part_size=320M icc_amp_heap_size=2M"				\
        " ddr_limit=2G ddr_heap_size=256M ddrcb_heap_size=64M"					\
	" cram_offset=0x25000 noswap nopcie"		                                        \
	" reth_on=1 ipa_oui=${ipa_oui}"								\
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
#define IPABOOT_COMMAND										\
	"if test -n \"${ipa_oui}\" -a -n \"${ipa_serial}\" -a -n \"${ipa_hwchar}\" -a "		\
		"-n \"${ipa_pai}\" -a -n \"${ipa_secmode}\" -a "				\
		"-n \"${ipa_loader_revocation}\" -a -n \"${ipa_app_revocation}\" -a "		\
		"-n \"${ethaddr}\" -a -n \"${eth1addr}\"; then "				\
	"  mtdparts default; "									\
	"  run select_bootargs; "								\
	"  run select_config; "									\
	"  setenv fsactive fs1; "								\
	"  setenv fsstandby fs0; "								\
	"  if ubi part FS; then "								\
	"    if ubifsmount ubi0:$fsactive; then "						\
	"      if ubifsload $loadaddr primary.flag; then "					\
	"        if ubifsload $slave_fitimage_loadaddr $slave_fitimage_name; then "		\
	"          run pcie_command; "	 							\
	"        fi; "										\
	"        if ubifsload $loadaddr fitImage; then "					\
	"          setenv bootargs $bootargs fsactive=$fsactive fsstandby=$fsstandby; "		\
	"          run secureboot; "								\
	"        fi; "										\
	"      fi; "										\
	"    fi; "										\
	"    setenv fsactive fs0; "								\
	"    setenv fsstandby fs1; "								\
	"    if ubifsmount ubi0:$fsactive; then "						\
	"      if ubifsload $slave_fitimage_loadaddr $slave_fitimage_name; then "               \
	"        run pcie_command; "	                                                        \
	"      fi; "                                                                            \
	"      if ubifsload $loadaddr fitImage; then "						\
	"        setenv bootargs $bootargs fsactive=$fsactive fsstandby=$fsstandby; "		\
	"        run secureboot; "								\
	"      fi; "										\
	"    fi; "										\
	"  fi; "										\
	"else "											\
	"  echo \"This board has not been characterised yet.  Please set up all required "	\
		"environment variables, save the environment and reboot.\"; "			\
	"fi; "											\
	"echo \"ERROR: Failed to load and run a kernel on this board, doh!\"; "

#define SET_BOOTARGS										\
	"setenv bootargs " CMD_LINE_ARGS_LINUX

#define SELECT_CONFIG										\
	"setenv selected_config config@1; "

#define PCIE_COMMAND "pcie 4 C I S W;"

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
	"secureboot=" SECURE_BOOT_COMMAND "\0"							\
	"pcie_command=" PCIE_COMMAND "\0"							\
	"slave_fitimage_name=fitImage\0"							\
	"slave_fitimage_loadaddr=0x0\0"

#define CONFIG_BOOTCOMMAND									\
	"run ipaboot"

#endif
