#ifndef __CONFIG_IPAT2K_NAND_H                                                                                    
#define __CONFIG_IPAT2K_NAND_H

#include <linux/sizes.h>
#define CONFIG_IPAT2K
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

/*SPI Flash Stuff*/
#if 0
#define CONFIG_SPI_FLASH
#define CONFIG_SPI_FLASH_STMICRO
#define CONFIG_CMD_SF
#endif

#define CONFIG_SYS_MALLOC_LEN   (CONFIG_ENV_SIZE + 1024*1024)
#define CONFIG_SYS_LOAD_ADDR	0x02000000
#define CONFIG_LOADADDR		0x02000000

#ifndef CONFIG_CMDLINE_TAG
#define CONFIG_CMDLINE_TAG
#endif
#ifndef CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_SETUP_MEMORY_TAGS
#endif
#ifndef CONFIG_INITRD_TAG
#define CONFIG_INITRD_TAG
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

#define MTD_PARTITION_DEFAULT	"nand0,0"
#define MTDIDS_DEFAULT		"nand0=gen_nand"
#define MTDPARTS_DEFAULT		\
	"mtdparts=gen_nand:"		\
	"1024K@0K(ML0),"		\
	"128K@1152K(RAW0),"		\
	"1024K@1280K(ML1),"		\
	"128K@2432K(RAW1),"		\
	"1024K@2560K(UBOOT0),"		\
	"128K@3712K(RAW2),"		\
	"1024K@3840K(UBOOT1),"		\
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
 * created from the file ipa9131.dts.  This simply contains a
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
#define CONFIG_BOOTCOUNT_EEPROM_ADDR CONFIG_SYS_I2C_EEPROM_ADDR 
#define CONFIG_BOOTCOUNT_IPAT2K
#define CONFIG_BOOTCOUNT_IPAT2K_MODULUS (CONFIG_BOOTCOUNT_LIMIT_COUNT*2)
/* Offset with in the EEPROM */
#define CONFIG_BOOTCOUNT_IPAT2K_OFFSET 65532

/* Auto-boot options */
//#define CONFIG_AUTOBOOT_KEYED
//#define CONFIG_AUTOBOOT_STOP_STR "stop"
//#define CONFIG_AUTOBOOT_PROMPT "ipaT2K: autoboot in %d seconds (\"stop\" to stop)\n",bootdelay

#define CONFIG_CHARACTERISATION_IPAT2K
#define CONFIG_CHARACTERISATION_EEPROM_ADDR CONFIG_SYS_I2C_EEPROM_ADDR
#define CONFIG_CHARACTERISATION_IPAT2K_OFFSET 60000
#define CONFIG_CHARACTERISATION_IPAT2K_SIZE 256
#define CONFIG_CHARACTERISATION_IPAT2K_VERSION 0x0

#define CONFIG_CMD_CHARACTERISE_HW

/* Monitor Command Prompt */
#undef CONFIG_SYS_PROMPT
#define CONFIG_SYS_PROMPT   "ipat2k=> "

/*
 * IPA Commands
 */
#define CONFIG_CMD_KEY
#define CONFIG_CMD_SECPARM
#define CONFIG_CMD_LIE
//#define CONFIG_CMD_LEDSET
//#define CONFIG_CMD_LEDC


#define CONFIG_ETHPRIME		"gemac1"
#define LINUX_CONSOLEDEV	"ttyS0"
#define CMD_LINE_ARGS_LINUX									\
	"console=" LINUX_CONSOLEDEV "," __stringify(CONFIG_BAUDRATE) "n8"			\
	" elevator=noop "									\
	MTDPARTS_DEFAULT									\
	" mem=380M"										\
	" hwaddress=eth1,${ethaddr},eth2,${eth1addr}"						\
	" icc_heap_size=130M icc_part_size=176M ddr_limit=2G ddr_heap_size=96M"                 \
	" cram_offset=0x25000 noswap nopcie"		                                        \
	" reth_on=1 ipa_oui=${ipa_oui}"								\
	" ipa_serial=${ipa_serial}"								\
	" ipa_hwchar=${ipa_hwchar}"								\
	" ipa_pai=${ipa_pai}"									\
	" ipa_secmode=${ipa_secmode}"								\
	" ipa_loader_revocation=${ipa_loader_revocation}"					\
	" ipa_app_revocation=${ipa_app_revocation}"


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
         "reset; "								\
        "fi; "									\
       "fi; "									\
      "fi; "									\
     "fi; "									\
    "fi; "									\
    "reset"

#define STANDARD_BOOT_COMMAND 									\
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
	"echo \"ERROR: Failed to load and run a kernel on this board, doh!\"; "

#define FALLBACK_BOOT_COMMAND                                                                   \
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
	"altbootcmd=" FALLBACK_BOOT_COMMAND "\0"				                \
	"bootlimit=" __stringify(CONFIG_BOOTCOUNT_LIMIT_COUNT) "\0"		                \
	"secureboot=" SECURE_BOOT_COMMAND "\0"

#define CONFIG_BOOTCOMMAND STANDARD_BOOT_COMMAND

#endif
