#ifndef __IPA9131_CONFIG_H
#define __IPA9131_CONFIG_H

#define CONFIG_IPA9131
#define CONFIG_BSC9131RDB
#define CONFIG_NAND
#define CONFIG_SYS_CLK_100

#include "BSC9131RDB.h"

#undef TSEC2_PHY_ADDR
#define TSEC2_PHY_ADDR		1

#undef CONFIG_SYS_MALLOC_LEN
#define CONFIG_SYS_MALLOC_LEN           (4 * 1024 * 1024) /*Reserved for malloc*/

#undef CONFIG_FIT_VERBOSE

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
    "128k@3200k(BLOB0),"        \
	"128k@3328k(BLOB1),"		\
	"127488k@3456k(FS),"		\
	"128k@130944k(BLOB2)"

/* we'd like U-Boot to pass partitioning to the kernel */
#define CONFIG_FDT_FIXUP_PARTITIONS

/*
 * We will support UBI volumes
 */
#define CONFIG_CMD_UBI
#define CONFIG_RBTREE
#define CONFIG_UBI_SILENCE_MSG

/*
 * We will support UBIFS filesystems
 */
#define CONFIG_CMD_UBIFS
#define CONFIG_LZO
#define CONFIG_UBIFS_SILENCE_MSG


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

/* Auto-boot options */
#define CONFIG_AUTOBOOT_KEYED
#define CONFIG_AUTOBOOT_STOP_STR "stop"
#define CONFIG_AUTOBOOT_PROMPT "ipa9131s: autoboot in %d seconds (\"stop\" to stop)\n",bootdelay

#define CONFIG_CHARACTERISATION_IPA9131
#define CONFIG_CHARACTERISATION_EEPROM_ADDR SPD_EEPROM_ADDRESS
#define CONFIG_CHARACTERISATION_IPA9131_OFFSET 0
#define CONFIG_CHARACTERISATION_IPA9131_SIZE 256
#define CONFIG_CHARACTERISATION_IPA9131_VERSION 0x1
#define CONFIG_IPA9131_MISC_FLAGS_OFFSET 239

#define CONFIG_CMD_CHARACTERISE_HW

#define CONFIG_MISC_INIT_R

/* Use 'long' help messages */
#define CONFIG_SYS_LONGHELP

/* Monitor Command Prompt */
#define CONFIG_SYS_PROMPT   "ipa9131s=> "



/*
 * IPA Commands
 */
#define CONFIG_CMD_KEY
#define CONFIG_CMD_SECPARM
#define CONFIG_CMD_LIE
#define CONFIG_CMD_LEDSET
#define CONFIG_CMD_LEDC
/* temporary - for easy fuse development */
#define CONFIG_CMD_IPA9131_FUSE



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
#undef CONFIG_ENV_IS_IN_NAND
#undef CONFIG_SYS_EXTRA_ENV_RELOC
#undef CONFIG_ENV_SIZE
#undef CONFIG_ENV_OFFSET
#undef CONFIG_ETHPRIME
#undef CONFIG_ENV_RANGE

#define CONFIG_ENV_IS_NOWHERE
#define CONFIG_ENV_ADDR	(CONFIG_SYS_MONITOR_BASE - 0x1000)
#define CONFIG_ENV_SIZE	0x2000 /* 8k for the environment - should be enough */

#define CONFIG_SILENT_CONSOLE
/* this board uses eth1 only - eth0 faces into the plastics */
#define CONFIG_ETHPRIME "eTSEC2"

#define LINUX_CONSOLEDEV "ttyS0"
#define IPA_BASE_BOOTARGS							\
	"rdinit=/init " 							\
	"elevator=noop "							\
	"ramdisk_size=600000 "							\
	"default_hugepagesz=256m "						\
	"hugepagesz=256m "							\
	"hugepages=1 "								\
	"max_num_ipc_channels=64 "						\
	"max_channel_depth=16 "							

#define CMDLINE_ARGS_LINUX IPA_BASE_BOOTARGS "console=" LINUX_CONSOLEDEV ","  __stringify(CONFIG_BAUDRATE) " "
#define CMDLINE_ARGS_LINUX_SILENT IPA_BASE_BOOTARGS "console=tty0 " "quiet"

#define SET_BOOTARGS								\
   "if silent_mode_enabled; then"						\
   " setenv bootargs " CMDLINE_ARGS_LINUX_SILENT ";"				\
   "else"									\
   " setenv bootargs " CMDLINE_ARGS_LINUX ";"					\
   "fi;"

#define SECURE_BOOT_COMMAND							\
    "NO_SEC_OK=0; "								\
    "DEV_KEY_OK=0; "								\
    "TST_KEY_OK=0; "								\
    "PRD_KEY_OK=0; "								\
    "secparm blank; "								\
    "if test $? -eq 0; then "							\
     "NO_SEC_OK=1; "								\
    "fi; "									\
    "secparm spcmode; "								\
    "if test $? -eq 0; then "							\
     "NO_SEC_OK=1; "								\
    "fi; "									\
    "secparm devmode; "								\
    "if test $? -eq 0; then "							\
     "DEV_KEY_OK=1; "								\
     "TST_KEY_OK=1; "								\
     "PRD_KEY_OK=1; "								\
    "fi; "									\
    "secparm tstmode; "								\
    "if test $? -eq 0; then "							\
     "TST_KEY_OK=1; "								\
     "PRD_KEY_OK=1; "								\
    "fi; "									\
    "secparm prdmode; "								\
    "if test $? -eq 0; then "							\
     "PRD_KEY_OK=1; "								\
    "fi; "									\
    "if test $PRD_KEY_OK -eq 1; then "						\
     "if key unrequire dev; then "						\
      "if key unrequire tstoem0; then "						\
       "if key require ipaoem0 conf; then "					\
        "if bootm ${loadaddr}#${selected_config}; then "			\
         "ledc all green red 3 1000; "						\
         "reset; "								\
        "fi; "									\
       "fi; "									\
      "fi; "									\
     "fi; "									\
    "fi; "									\
    "if test $TST_KEY_OK -eq 1; then "						\
     "if key unrequire dev; then "						\
      "if key unrequire ipaoem0; then "						\
       "if key require tstoem0 conf; then "					\
        "if bootm ${loadaddr}#${selected_config}; then "			\
         "ledc all green red 3 1000; "						\
         "reset; "								\
        "fi; "									\
       "fi; "									\
      "fi; "									\
     "fi; "									\
    "fi; "									\
    "if test $DEV_KEY_OK -eq 1; then "						\
     "if key unrequire tstoem0; then "						\
      "if key unrequire ipaoem0; then "						\
       "if key require dev conf; then "						\
        "if bootm ${loadaddr}#${selected_config}; then "			\
         "ledc all green red 3 1000; "						\
         "reset; "								\
        "fi; "									\
       "fi; "									\
      "fi; "									\
     "fi; "									\
    "fi; "									\
    "if test $NO_SEC_OK -eq 1; then "						\
     "if key unrequire tstoem0; then "						\
      "if key unrequire ipaoem0; then "						\
       "if key unrequire dev; then "						\
        "if bootm ${loadaddr}#${selected_config}; then "			\
         "ledc all green red 3 1000; "						\
         "reset; "								\
        "fi; "									\
       "fi; "									\
      "fi; "									\
     "fi; "									\
    "fi; "									\
    "ledc all green red 3 1000; "						\
    "reset"

/*
 * This command block is responsible for selecting the correct FDT to pass to
 * Linux based on the characterised board type.  The "current" state of the art
 * is the default, and older FDTs are selected based on the board variant part
 * number (245, 248, 278 etc.)
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
    "ledc all green off 1 300; "						\
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
    "ledc all green red 3 1000; "						\
    "reset"

#define FALLBACK_BOOT_COMMAND							\
    "ledc all green off 1 300; "						\
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
    "ledc all green red 3 1000; "						\
    "reset"

#define CONFIG_EXTRA_ENV_SETTINGS						\
	"netdev=eth1\0"								\
	"ethrotate=no\0"							\
	"loadaddr=1000000\0"							\
	"consoledev=ttyS0\0"							\
	"select_bootargs=" SET_BOOTARGS "\0"					\
	"select_config=" SELECT_CONFIG_COMMAND "\0"				\
	"altbootcmd=" FALLBACK_BOOT_COMMAND "\0"				\
	"bootlimit=" __stringify(CONFIG_BOOTCOUNT_LIMIT_COUNT) "\0"		\
	"secureboot=" SECURE_BOOT_COMMAND "\0"                                  \
	"bootm_size=0x37000000\0"

#define CONFIG_BOOTCOMMAND STANDARD_BOOT_COMMAND
#endif
