#ifndef __IPA9131_CONFIG_H
#define __IPA9131_CONFIG_H

#define CONFIG_IPA9131
#define CONFIG_BSC9131RDB
#define CONFIG_NAND
#define CONFIG_SYS_CLK_100

#include "BSC9131RDB.h"

/* #undef CONFIG_FIT_VERBOSE */

/*
 * U-Boot execution will be controlled from an embedded OF tree
 * created from the file ipa9131.dts.  This simply contains a
 * set of security keys.
 */
#define CONFIG_OF_CONTROL
#define CONFIG_OF_EMBED
#define CONFIG_DEFAULT_DEVICE_TREE "ipa9131"

/*
 * We will be doing RSA FIT signatures for secure boot.
 */
#define CONFIG_FIT_SIGNATURE
#define CONFIG_RSA
/* IPA-specific: don't do signature fallback */
#define CONFIG_FIT_SIGNATURE_REQUIRED_KEYNODE_ONLY

/*
 * We will do FIT image revocation checking (IPA specific)
 */
#define CONFIG_FIT_REVOCATION


/*
 * Add a nand write command to trim 0xff pages (required for UBI)
 */
#define CONFIG_CMD_NAND_TRIMFFS

/*
 * Needed for UBI and command-line partitioning
 */
#define CONFIG_MTD_DEVICE
#define CONFIG_MTD_PARTITIONS
#define CONFIG_CMD_MTDPARTS

#define MTD_PARTITION_DEFAULT	"nand0,0"
#define MTDIDS_DEFAULT		"nand0=ff800000.flash"
#define MTDPARTS_DEFAULT		\
	"mtdparts=ff800000.flash:"	\
	"1024k@0k(ML),"			\
	"1024k@1024k(UBOOT0),"		\
	"1024k@2048k(UBOOT1),"		\
	"128k@3328k(BLOB0),"		\
	"127488k@3456k(FS),"		\
	"128k@130944k(BLOB1)"

/* we'd like U-Boot to pass partitioning to the kernel */
#define CONFIG_FDT_FIXUP_PARTITIONS

/*
 * We will support UBI volumes
 */
#define CONFIG_CMD_UBI
#define CONFIG_RBTREE
/* #define CONFIG_UBI_SILENCE_MSG */

/*
 * We will support UBIFS filesystems
 */
#define CONFIG_CMD_UBIFS
#define CONFIG_LZO
/* #define CONFIG_UBIFS_SILENCE_MSG */


/*
 * Bootcount is stored in the SPD EEPROM as it is nolonger used
 * for DDR timings.
 */
#define CONFIG_BOOTDELAY 3
#define CONFIG_BOOTCOUNT_LIMIT
#define CONFIG_BOOTCOUNT_LIMIT_COUNT 4
#define CONFIG_BOOTCOUNT_EEPROM_ADDR SPD_EEPROM_ADDRESS
#define CONFIG_BOOTCOUNT_IPA9131
#define CONFIG_BOOTCOUNT_IPA9131_MODULUS (CONFIG_BOOTCOUNT_LIMIT_COUNT*2)
/* Offset with in the EEPROM */
#define CONFIG_BOOTCOUNT_IPA9131_OFFSET 255

#define CONFIG_CHARACTERISATION_IPA9131
#define CONFIG_CHARACTERISATION_EEPROM_ADDR SPD_EEPROM_ADDRESS
#define CONFIG_CHARACTERISATION_IPA9131_OFFSET 0
#define CONFIG_CHARACTERISATION_IPA9131_SIZE 31
#define CONFIG_CHARACTERISATION_IPA9131_VERSION 0x0

#define CONFIG_CMD_CHARACTERISE_HW

#define CONFIG_MISC_INIT_R




/*
 * IPA Commands
 */
#define CONFIG_CMD_KEY
#define CONFIG_CMD_SECPARM
#define CONFIG_CMD_LIE




/*
 * Automatic Boot Scripts and Environment (temporary for now)
 */

#undef CONFIG_HOSTNAME
#undef CONFIG_ROOTPATH
#undef CONFIG_BOOTFILE
#undef CONFIG_UBOOTPATH
#undef CONFIG_EXTRA_ENV_SETTINGS
#undef CONFIG_RAMBOOTCOMMAND
#undef CONFIG_BOOTCOMMAND

#define IPA_BASE_BOOTARGS							\
	"rdinit=/init " 							\
	"elevator=noop "							\
	"ramdisk_size=600000 "							\
	"default_hugepagesz=256m "						\
	"hugepagesz=256m "							\
	"hugepages=1 "								\
	"max_num_ipc_channels=64 "						\
	"max_channel_depth=16 "

#define LINUX_CONSOLEDEV "ttyS0"
#define CMDLINE_ARGS_LINUX IPA_BASE_BOOTARGS "console=" LINUX_CONSOLEDEV ","  __stringify(CONFIG_BAUDRATE)
#define CMDLINE_ARGS_LINUX_SILENT IPA_BASE_BOOTARGS "console="

#define SET_BOOTARGS								\
   "if false; then"								\
   " setenv bootargs " CMDLINE_ARGS_LINUX_SILENT ";"				\
   "else"									\
   " setenv bootargs " CMDLINE_ARGS_LINUX ";"					\
   "fi;"

/* temporary */
#define SECURE_BOOT_COMMAND							\
    "bootm ${loadaddr}#${selected_config}; "					\
    "reset"

/*
 * This command block is responsible for selecting the correct FDT to pass to
 * Linux based on the characterised board type.  The "current" state of the art
 * is the default, and older FDTs are selected based on the board variant part
 * number (245, 248 etc.)
 *
 * This sets up the 'selected_config' environment variable, which is used by
 * the secure boot command to pass as a config selector to the bootm command.
 */
#define SELECT_CONFIG_COMMAND							\
    "setenv selected_config config@2; "						\
    "if test -n \"${board_variant_part}\"; then "				\
     "if test ${board_variant_part} -eq 245; then "				\
      "setenv selected_config config@1; "					\
     "fi; "									\
    "fi"

#define STANDARD_BOOT_COMMAND							\
    "mtdparts default; "							\
    "run select_bootargs; "							\
    "run select_config; "							\
    "setenv fsactive fs1; "							\
    "setenv fsstandby fs0; "							\
    "if ubi part FS; then "							\
     "if ubifsmount ubi0:$fsactive; then "					\
      "if ubifsload $loadaddr primary.flag; then "				\
       "if ubifsload $loadaddr fitImage; then "					\
        "setenv bootargs $bootargs fsactive=$fsactive fsstandby=$fsstandby; "	\
        "run secureboot; "							\
       "fi; "									\
     "else "									\
       "setenv fsactive fs0; "							\
       "setenv fsstandby fs1; "							\
       "if ubifsmount ubi0:$fsactive; then "					\
         "if ubifsload $loadaddr fitImage; then "				\
          "setenv bootargs $bootargs fsactive=$fsactive fsstandby=$fsstandby; "	\
          "run secureboot; "							\
         "fi; "									\
       "fi; "									\
      "fi; "									\
     "fi; "									\
    "fi; "									\
    "reset"

#define FALLBACK_BOOT_COMMAND							\
    "mtdparts default; "							\
    "run select_bootargs; "							\
    "run select_config; "							\
    "setenv fsactive fs1; "							\
    "setenv fsstandby fs0; "							\
    "if ubi part FS; then "							\
     "if ubifsmount ubi0:$fsactive; then "					\
      "if ubifsload $loadaddr primary.flag; then "				\
       "setenv fsactive fs0; "							\
       "setenv fsstandby fs1; "							\
       "if ubifsmount ubi0:$fsactive; then "					\
         "if ubifsload $loadaddr fitImage; then "				\
          "setenv bootargs $bootargs fsactive=$fsactive fsstandby=$fsstandby; "	\
          "run secureboot; "							\
         "fi; "									\
       "fi; "									\
     "else "									\
       "if ubifsload $loadaddr fitImage; then "					\
        "setenv bootargs $bootargs fsactive=$fsactive fsstandby=$fsstandby; "	\
        "run secureboot; "							\
       "fi; "									\
      "fi; "									\
     "fi; "									\
    "fi; "									\
    "reset"

#define CONFIG_EXTRA_ENV_SETTINGS						\
	"netdev=eth0\0"								\
	"loadaddr=1000000\0"							\
	"consoledev=ttyS0\0"							\
	"select_bootargs=" SET_BOOTARGS "\0"					\
	"select_config=" SELECT_CONFIG_COMMAND "\0"				\
	"altbootcmd=" FALLBACK_BOOT_COMMAND "\0"				\
	"bootlimit=" __stringify(CONFIG_BOOTCOUNT_LIMIT_COUNT) "\0"		\
	"secureboot=" SECURE_BOOT_COMMAND "\0"

#define CONFIG_BOOTCOMMAND STANDARD_BOOT_COMMAND

#endif
