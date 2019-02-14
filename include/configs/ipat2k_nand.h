#ifndef __CONFIG_IPAT2K_NAND_H                                                                                    
#define __CONFIG_IPAT2K_NAND_H

#include <linux/sizes.h>
#include <asm/hardware.h>
#define CONFIG_IPAT2K
#include "m82xxx_gt_nand.h"

#if defined(DEBUG)
#undef DEBUG
#endif

#undef CONFIG_SYS_TEXT_BASE
#undef CONFIG_ENV_IS_IN_FLASH
#undef CONFIG_ENV_OFFSET
#undef CONFIG_SYS_MALLOC_LEN
#undef CONFIG_SYS_LOAD_ADDR
#undef CONFIG_LOADADDR
#undef CONFIG_BOOTCOMMAND
#undef CONFIG_BOOTARGS
#undef CONFIG_ETHADDR
#undef CONFIG_BOOTFILE
#undef CONFIG_OVERWRITE_ETHADDR_ONCE
#undef CONFIG_SYS_NS16550_REG_SIZE
#undef CONFIG_SYS_PBSIZE
#undef CONFIG_CMD_T3300
#undef CONFIG_CMD_PCIE
#undef CONFIG_ADT75
#undef CONFIG_EXTRA_ENV_SETTINGS
#undef CONFIG_IDENT_STRING
#if 0
/*SPI Flash Stuff*/
#define CONFIG_SPI_FLASH
#define CONFIG_SPI_FLASH_STMICRO
#define CONFIG_CMD_SF
#endif
#define CONFIG_IDENT_STRING     MINDSPEED_VERSION "\n\nIPAS60-UBOOT\n"
#define CONFIG_CMD_DHCP
#define CONFIG_SYS_TEXT_BASE 0x20000000
#define CONFIG_SYS_INIT_SP_ADDR (IRAM_BASE + 0x800)
#define CONFIG_SYS_MALLOC_LEN   (4*1024*1024)
#define CONFIG_SYS_LOAD_ADDR	0x02000000
#define CONFIG_LOADADDR		0x02000000
#define CONFIG_SYS_PBSIZE    (CONFIG_SYS_CBSIZE+16)
#define CONFIG_USE_PRIVATE_LIBGCC
#define CONFIG_SYS_NS16550_REG_SIZE     -4

#ifndef CONFIG_CMDLINE_TAG
#define CONFIG_CMDLINE_TAG
#endif
#ifndef CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_SETUP_MEMORY_TAGS
#endif
#ifndef CONFIG_INITRD_TAG
#define CONFIG_INITRD_TAG
#endif

#define CONFIG_SYS_SDRAM_BASE DDR0_BASE
#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_CMD_ECHO

#define NAND_FLASH_SECTOR_SIZE      (SZ_128K)
#define CONFIG_ENV_OFFSET           (NAND_FLASH_SECTOR_SIZE * 8)
#define CONFIG_ENV_OFFSET_REDUND    (NAND_FLASH_SECTOR_SIZE * 18)
#define CONFIG_ENV_OVERWRITE

#ifndef CONFIG_CMD_NAND
#define CONFIG_CMD_NAND
#endif
#define CONFIG_CMD_NAND_TRIMFFS

#define CONFIG_MTD_DEVICE
#define CONFIG_CMD_MTDPARTS
#define CONFIG_MTD_PARTITIONS

#define MTD_PARTITION_DEFAULT	"nand0,0"
#define MTDIDS_DEFAULT		"nand0=gen_nand"
#define MTDPARTS_DEFAULT		\
	"mtdparts=gen_nand:"		\
	"1024K@0K(UBOOT0),"		\
	"128K@1152K(RAW0),"		\
	"1024K@1280K(UBOOT1),"		\
	"128K@2432K(RAW1),"		\
	"1024K@2560K(RES0),"		\
	"128K@3712K(RAW2),"		\
	"1024K@3840K(RES1),"		\
	"128K@4992K(RAW3),"		\
	"-@5120K(FS)"

#define CONFIG_CMD_UBI
// #define CONFIG_UBI_SILENCE_MSG
#define CONFIG_RBTREE

#define CONFIG_CMD_UBIFS
#define CONFIG_LZO
// #define CONFIG_UBIFS_SILENCE_MSG

#define CONFIG_FIT

/*
 * U-Boot execution will be controlled from an embedded OF tree
 * created from the file ipat2k.dts.  This simply contains a
 * set of security keys.
 */
#define CONFIG_OF_CONTROL
#define CONFIG_OF_EMBED
#define CONFIG_DEFAULT_DEVICE_TREE "ipat2k"
#define CONFIG_OF_LIBFDT

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
 * Bootcount is stored in the I2C EEPROM.
*/
#define CONFIG_BOOTDELAY 3
#define CONFIG_BOOTCOUNT_LIMIT
#define CONFIG_BOOTCOUNT_LIMIT_COUNT 4
#define CONFIG_BOOTCOUNT_EEPROM_ADDR 0x53
#define CONFIG_BOOTCOUNT_IPAT2K
#define CONFIG_BOOTCOUNT_IPAT2K_MODULUS (CONFIG_BOOTCOUNT_LIMIT_COUNT*2)
/* Offset with in the EEPROM */
#define CONFIG_BOOTCOUNT_IPAT2K_OFFSET (63*1024 + 512)

/* Auto-boot options */
#define CONFIG_AUTOBOOT_KEYED
#define CONFIG_AUTOBOOT_STOP_STR "stop"
#define CONFIG_AUTOBOOT_PROMPT "ipas60: autoboot in %d seconds (\"stop\" to stop)\n",bootdelay

#define CONFIG_CHARACTERISATION_IPAT2K
#define CONFIG_CHARACTERISATION_EEPROM_ADDR 0x53
#define CONFIG_CHARACTERISATION_IPAT2K_OFFSET (63 * 1024)
#define CONFIG_CHARACTERISATION_IPAT2K_SIZE 256
#define CONFIG_CHARACTERISATION_IPAT2K_VERSION 0x0

#define CONFIG_CHARACTERISATION_IPA_RAD_SIZE 256
#define CONFIG_CHARACTERISATION_IPA_RAD_VERSION 0x0
#define CONFIG_CHARACTERISATION_IPA_RAD_OFFSET 0
#define CONFIG_CHARACTERISATION_RAD_EEPROM_ADDR 0x57
#define CONFIG_CHARACTERISATION_RAD_EEPROM_PSIZE 64
#define CONFIG_CMD_CHARACTERISE_HW

/* Monitor Command Prompt */
#undef CONFIG_SYS_PROMPT
#define CONFIG_SYS_PROMPT   "ipas60=> "

/*
 * IPA Commands
 */
#define CONFIG_CMD_KEY
#define CONFIG_CMD_SECPARM
#define CONFIG_CMD_LIE
#define CONFIG_CMD_LEDSET
#define CONFIG_CMD_LEDC
//Keeping watchdog disabled for now, as the u-boot revision where this was added was never pulled in to yocto build.
//#define CONFIG_HW_WATCHDOG


#define CONFIG_ETHPRIME		"gemac1"
#define LINUX_CONSOLEDEV	"ttyS0"
#define IPA_BASE_BOOTARGS									\
	" elevator=noop "									\
	MTDPARTS_DEFAULT									\
	" mem=512M"										\
	" hwaddress=eth1,${ethaddr},eth2,${eth1addr}"						\
	" icc_heap_size=132M icc_part_size=320M icc_amp_heap_size=2M ddr_limit=2G"              \
	" ddr_heap_size=256M ddrcb_heap_size=64M cram_offset=0x26000 noswap nopcie"             \
	" reth_on=1 ethpoll=1000 pcb_assembly_issue=${board_pcb_assembly_issue}"                \
	" board_variant_part=${board_variant_part} bb_variant_part=${bb_variant_part}"          \
	" radio_variant_part=${radio_variant_part} radio_pcbai=${radio_pcbai}"                  \

#define CMDLINE_ARGS_LINUX IPA_BASE_BOOTARGS  " console=" LINUX_CONSOLEDEV "," __stringify(CONFIG_BAUDRATE) "n8"
#define CMDLINE_ARGS_LINUX_SILENT IPA_BASE_BOOTARGS " console=tty0 " "quiet"

#define SET_BOOTARGS							\
	"if silent_mode_enabled; then "					\
	" setenv bootargs " CMDLINE_ARGS_LINUX_SILENT "; "		\
	"else; "							\
	" setenv bootargs " CMDLINE_ARGS_LINUX "; "			\
	"fi; "


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
        "ledc all green red 3 1000;"						\
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
        "ledc all green red 3 1000;"						\
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
        "ledc all green red 3 1000;"						\
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
        "ledc all green red 3 1000;"						\
         "reset; "								\
        "fi; "									\
       "fi; "									\
      "fi; "									\
     "fi; "									\
    "fi; "									\
    "ledc all green red 3 1000;"						\
    "reset"

#define STANDARD_BOOT_COMMAND 									\
	"ledc nwk svc green off 1 300; "							\
	"mtdparts default; "									\
	"run select_bootargs; "								        \
	"run select_config; "									\
	"setenv fsactive fs1; "							         	\
	"setenv fsstandby fs0; "								\
	"if ubi part FS; then "								        \
	"  if ubifsmount ubi0:$fsactive; then "						        \
	"    if ubifsload $loadaddr primary.flag; then "					\
	"      if ubifsload $loadaddr fitImage; then "						\
	"         setenv bootargs $bootargs fsactive=$fsactive fsstandby=$fsstandby; "		\
	"         run secureboot; "								\
	"      fi; "										\
	"    fi; "										\
	"  fi; "										\
	"  setenv fsactive fs0; "								\
	"  setenv fsstandby fs1; "								\
	"  if ubifsmount ubi0:$fsactive; then "						        \
	"    if ubifsload $loadaddr fitImage; then "						\
	"      setenv bootargs $bootargs fsactive=$fsactive fsstandby=$fsstandby; "		\
	"      run secureboot; "								\
	"    fi; "										\
	"  fi; "										\
	"fi; "										        \
	"ledc all green red 3 1000;"								\
	"reset"

#define FALLBACK_BOOT_COMMAND                                                                   \
	"ledc nwk svc green off 1 300; "							\
	"mtdparts default; "									\
	"run select_bootargs; "								        \
	"run select_config; "									\
	"setenv fsactive fs1; "								        \
	"setenv fsstandby fs0; "								\
	"if ubi part FS; then "								        \
	" if ubifsmount ubi0:$fsactive; then "						        \
	"  if ubifsload $loadaddr primary.flag; then "                                          \
        "     setenv fsactive fs0; "							        \
        "     setenv fsstandby fs1; "                                                           \
        "     if ubifsmount ubi0:$fsactive; then "                                              \
	"      if ubifsload $loadaddr fitImage; then "					        \
	"          setenv bootargs $bootargs fsactive=$fsactive fsstandby=$fsstandby; "		\
	"          run secureboot; "								\
	"      fi; "										\
	"     fi; "										\
	"     else; "										\
	"      if ubifsload $loadaddr fitImage; then "						\
	"        setenv bootargs $bootargs fsactive=$fsactive fsstandby=$fsstandby; "		\
	"        run secureboot; "								\
	"      fi; "										\
	"     fi; "										\
	"  fi; "										\
	"fi; "										        \
	"ledc all green red 3 1000;"								\
	"reset"

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
	"altbootcmd=" FALLBACK_BOOT_COMMAND "\0"				                \
	"bootlimit=" __stringify(CONFIG_BOOTCOUNT_LIMIT_COUNT) "\0"		                \
	"secureboot=" SECURE_BOOT_COMMAND "\0"

#define CONFIG_BOOTCOMMAND STANDARD_BOOT_COMMAND
#define CONFIG_SILENT_CONSOLE
#define CONFIG_SILENT_CONSOLE_UPDATE_ON_SET
#define CONFIG_SILENT_CONSOLE_UPDATE_ON_RELOC
/*Silent u-boot only with CONFIG_SILENT_CONSOLE, for kernel seprate bootargs environment variable is set*/
#define CONFIG_SILENT_U_BOOT_ONLY
#endif
