#ifndef __CONFIG_IPAFSM92XX_H
#define __CONFIG_IPAFSM92XX_H

#include <asm/sizes.h>
#include <asm/mach-types.h>

/* #define DEBUG */

#define CONFIG_MACH_TYPE MACH_TYPE_FSM9XXX_SURF
#define CONFIG_IPAFSM92XX

/* SDRAM configuration */
#define CONFIG_SYS_SDRAM_BASE (0x00000000)
#define CONFIG_SYS_SDRAM_SIZE SZ_512M
#define CONFIG_NR_DRAM_BANKS (1)
#define PHYS_SDRAM_1 0x10000000 // Base Address for IPA
#define PHYS_SDRAM_1_SIZE SZ_256M


#define CONFIG_BITREVERSE
#define CONFIG_ENV_IS_NOWHERE
#define CONFIG_ENV_SIZE SZ_128K
#define CONFIG_SYS_GENERIC_BOARD
#define CONFIG_SYS_BAUDRATE_TABLE { 4800, 9600, 19200, 38400, 57600, 115200 }
#define CONFIG_BAUDRATE 115200

#define CONFIG_SYS_MALLOC_LEN SZ_8M
#define CONFIG_SYS_BOOTM_LEN SZ_16M

#define CONFIG_SYS_TEXT_BASE 0x18000000 /* 384M */

#define CONFIG_SYS_INIT_SP_ADDR (CONFIG_SYS_TEXT_BASE - (3*SZ_4M)) /* stack is at 372M, growing down and with global data above */

#define CONFIG_SYS_MAXARGS (16)
#define CONFIG_SYS_CBSIZE SZ_1K
#define CONFIG_SYS_BARGSIZE CONFIG_SYS_CBSIZE
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT_HUSH_PS2 "ipafsm92xx> "

#define CONFIG_SYS_LOAD_ADDR 0x1c000000 

#define CONFIG_SYS_HZ 1000

#define CONFIG_MSM_SERIAL
#define CONFIG_CONS_INDEX 1

#define CONFIG_MISC_INIT_R
#define CONFIG_SKIP_LOWLEVEL_INIT

#include "config_cmd_default.h"

#define CONFIG_SYS_PROMPT   "ipafsm92xx> "
#define CONFIG_BOOTDELAY 3
#define CONFIG_AUTOBOOT_KEYED
#define CONFIG_AUTOBOOT_STOP_STR "stop"
#define CONFIG_AUTOBOOT_PROMPT "autoboot in %d seconds (\"stop\" to stop)\n",bootdelay

#define CONFIG_CMD_NAND
#define CONFIG_CMD_FUSE
#define CONFIG_CMD_PM8058
#define CONFIG_CMD_KEY
#define CONFIG_CMD_SECPARM
#define CONFIG_CMD_LIE
 
/* Use 'long' help messages */
#define CONFIG_SYS_LONGHELP 

/* NAND Flash Memory Stuff */
#define CONFIG_SYS_NAND_SELF_INIT
#define CONFIG_SYS_MAX_FLASH_SECT 4096
#ifndef CONFIG_SYS_NAND_SELF_INIT
#define CONFIG_SYS_NAND_BASE      (0x0)
#endif

#define CONFIG_SYS_MAX_NAND_DEVICE  1
#define CONFIG_SYS_MAX_FLASH_BANKS  1

/* Enable command line MTD partitioning */
#define CONFIG_MTD_PARTITIONS
#define CONFIG_MTD_DEVICE
#define CONFIG_CMD_MTDPARTS

#define	MTD_PARTITION_DEFAULT	"nand0,0"
#define MTDIDS_DEFAULT          "nand0=msm_nand"
/* Uboot needs to access only FS partition */
#define MTDPARTS_DEFAULT        "mtdparts=msm_nand:184320k@77824k(FS)"

#define LINUX_CONSOLEDEV "ttyMSM0"
#define CMD_LINE_ARGS_LINUX	"rdinit=/init console=" LINUX_CONSOLEDEV ","  __stringify(CONFIG_BAUDRATE) " elevator=noop"
#define CMD_LINE_ARGS_LINUX_SILENT "rdinit=/init console=tty0 elevator=noop"

#define CONFIG_CMD_MD5SUM
#define CONFIG_MD5

/* Enable UBI/UBIFS */
#define CONFIG_CMD_UBI
#define CONFIG_CMD_UBIFS
#define CONFIG_LZO
#define CONFIG_RBTREE
/* fitImage location in UBIFS*/
#define UBI_PART "FS"
#define UBI_VOLUME "ubi0"
#define UBIFS_0 "fs0"
#define UBIFS_1 "fs1"
#define FIT_IMAGE "fitImage"

/* Enable FIT and OF control support */
#define CONFIG_OF_CONTROL
#define CONFIG_OF_EMBED
#define CONFIG_DEFAULT_DEVICE_TREE "ipafsm92xx"
#define CONFIG_OF_LIBFDT
#define CONFIG_FIT
#define CONFIG_FIT_SIGNATURE
#define CONFIG_FIT_REVOCATION
#define CONFIG_RSA

/* Enable ATAGS */
#define CONFIG_CMDLINE_TAG          1
#define CONFIG_SETUP_MEMORY_TAGS    1
#define CONFIG_INITRD_TAG           1

/* The boot command will set bootargs */
#undef  CONFIG_BOOTARGS

/* Unless specified here we'll just rely on the kernel default */
#define OTHERBOOTARGS

/* Define CONFIG_BOOTCOMMAND as */
#define CONFIG_BOOTCOMMAND  "run ubiboot"

#define CONFIG_BOOTCOUNT_LIMIT 4

#define SECURE_BOOT_COMMAND                       \
   "setenv bootargs $bootargs $othbootargs;"      \
   "setenv bootargs $bootargs $bootbankargs;"     \
   "if secparm secboot; then"                     \
   "  if secparm devmode; then"                   \
   "    if key unrequire ipaoem0; then"           \
   "      if key unrequire tstoem0; then"         \
   "        if key unrequire manoem0; then"       \
   "          if key require dev conf; then"      \
   "            if bootm $loadaddr; then"         \
   "              reset;"                         \
   "            fi;"                              \
   "          fi;"                                \
   "        fi;"                                  \
   "      fi;"                                    \
   "    fi;"                                      \
   "    if key unrequire ipaoem0; then"           \
   "      if key unrequire dev; then"             \
   "        if key unrequire manoem0; then"       \
   "          if key require tstoem0 conf; then"  \
   "            if bootm $loadaddr; then"         \
   "              reset;"                         \
   "            fi;"                              \
   "          fi;"                                \
   "        fi;"                                  \
   "      fi;"                                    \
   "    fi;"                                      \
   "    if key unrequire tstoem0; then"           \
   "      if key unrequire dev; then"             \
   "        if key unrequire manoem0; then"       \
   "          if key require ipaoem0 conf; then"  \
   "            if bootm $loadaddr; then"         \
   "              reset;"                         \
   "            fi;"                              \
   "          fi;"                                \
   "        fi;"                                  \
   "      fi;"                                    \
   "    fi;"                                      \
   "    if key unrequire tstoem0; then"           \
   "      if key unrequire dev; then"             \
   "        if key unrequire ipaoem0; then"       \
   "          if key require manoem0 conf; then"  \
   "            if bootm $loadaddr; then"         \
   "              reset;"                         \
   "            fi;"                              \
   "          fi;"                                \
   "        fi;"                                  \
   "      fi;"                                    \
   "    fi;"                                      \
   "  else"                                       \
   "    if key unrequire dev; then"               \
   "      if key unrequire tstoem0; then"         \
   "        if key unrequire manoem0; then"       \
   "          if key require ipaoem0 conf; then"  \
   "            if bootm $loadaddr; then"         \
   "              reset;"                         \
   "            fi;"                              \
   "          fi;"                                \
   "        fi;"                                  \
   "      fi;"                                    \
   "    fi;"                                      \
   "    if key unrequire dev; then"               \
   "      if key unrequire tstoem0; then"         \
   "        if key unrequire ipaoem0; then"       \
   "          if key require manoem0 conf; then"  \
   "            if bootm $loadaddr; then"         \
   "              reset;"                         \
   "            fi;"                              \
   "          fi;"                                \
   "        fi;"                                  \
   "      fi;"                                    \
   "    fi;"                                      \
   "  fi;"                                        \
   "else"                                         \
   "  key unrequire dev;"                         \
   "  key unrequire tstoem0;"                     \
   "  key unrequire ipaoem0;"                     \
   "  key unrequire manoem0;"                     \
   "  bootm $loadaddr;"                           \
   "fi;"                                          \
   "reset;"

#define TEST_BOOT_COUNT_EXCEEDED                                    \
   "if bootcount_combined_exceeded; then"                           \
   " run bootfailedhang;"                                           \
   "fi;"

#define SET_BOOTARGS                                                \
   "if silent_mode_enabled; then"                                   \
   " setenv bootargs " CMD_LINE_ARGS_LINUX_SILENT ";"               \
   "else"                                                           \
   " setenv bootargs " CMD_LINE_ARGS_LINUX ";"                      \
   "fi;"                                                            \


#define UBIBOOT_COMMAND                                             \
   TEST_BOOT_COUNT_EXCEEDED                                         \
   SET_BOOTARGS                                                     \
   "setenv fsactive " UBIFS_1 ";"                                   \
   "setenv fsstandby " UBIFS_0 ";"                                  \
   "ubi part " UBI_PART ";"                                         \
   "ubifsmount " UBI_VOLUME ":$fsactive;"                           \
   "if ubifsload $loadaddr primary.flag; then"                      \
   " echo \"Normal boot from $fsactive\";"                          \
   "else"                                                           \
   " setenv fsactive " UBIFS_0 ";"                                  \
   " setenv fsstandby " UBIFS_1 ";"                                 \
   " ubifsmount " UBI_VOLUME ":$fsactive;"                          \
   " echo \"Normal boot from $fsactive\";"                          \
   "fi;"                                                            \
   "if ubifsload $loadaddr " FIT_IMAGE "; then"                     \
   " setenv bootbankargs fsactive=$fsactive fsstandby=$fsstandby;"  \
   " run secureboot;"                                               \
   "fi;"                                                            \
   "reset;"

#define UBIALTBOOT_COMMAND                                          \
   TEST_BOOT_COUNT_EXCEEDED                                         \
   SET_BOOTARGS                                                     \
   "setenv fsactive " UBIFS_1 ";"                                   \
   "setenv fsstandby " UBIFS_0 ";"                                  \
   "ubi part " UBI_PART ";"                                         \
   "ubifsmount " UBI_VOLUME ":$fsactive;"                           \
   "if ubifsload $loadaddr primary.flag; then"                      \
   " setenv fsactive " UBIFS_0 ";"                                  \
   " setenv fsstandby " UBIFS_1 ";"                                 \
   " ubifsmount " UBI_VOLUME ":$fsactive;"                          \
   " echo \"Fallback boot from $fsactive\";"                        \
   "else"                                                           \
   " echo \"Fallback boot from $fsactive\";"                        \
   "fi;"                                                            \
   "if ubifsload $loadaddr " FIT_IMAGE "; then"                     \
   " setenv bootbankargs fsactive=$fsactive fsstandby=$fsstandby;"  \
   " run secureboot;"                                               \
   "fi;"                                                            \
   "reset;"

#define BOOT_FAILED_HANG_COMMAND                                    \
    "led all off; while true ; do sleep 1; done"

#define CONFIG_EXTRA_ENV_SETTINGS                                           \
   "consoledev=" LINUX_CONSOLEDEV "\0"                                      \
   "bootlimit=" __stringify(CONFIG_BOOTCOUNT_LIMIT) "\0"                    \
   "loadaddr=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0"                       \
   "othbootargs=" __stringify (OTHERBOOTARGS) "\0"                          \
   "mtdids=" MTDIDS_DEFAULT "\0"                                            \
   "mtdparts=" MTDPARTS_DEFAULT "\0"                                        \
   "secureboot=" SECURE_BOOT_COMMAND "\0"                                   \
   "bootfailedhang=" BOOT_FAILED_HANG_COMMAND "\0"                          \
   "ubiboot=" UBIBOOT_COMMAND "\0"                                          \
   "ubialtboot=" UBIALTBOOT_COMMAND "\0"                                    \
   "altbootcmd=run ubialtboot;\0"

/* LEDs */
#define CONFIG_CMD_LED
#define CONFIG_STATUS_LED           1
#define CONFIG_BOARD_SPECIFIC_LED   1
#define STATUS_LED_BOOT             0
#define STATUS_LED_BIT              0x1
#define STATUS_LED_STATE            STATUS_LED_OFF
#define STATUS_LED_PERIOD           (CONFIG_SYS_HZ / 2)

#define STATUS_LED_BIT1             0x02
#define STATUS_LED_STATE1           STATUS_LED_OFF
#define STATUS_LED_PERIOD1          (CONFIG_SYS_HZ / 2)

#define STATUS_LED_BIT2             0x04
#define STATUS_LED_STATE2           STATUS_LED_OFF
#define STATUS_LED_PERIOD2          (CONFIG_SYS_HZ / 2)

#define STATUS_LED_BIT3             0x08
#define STATUS_LED_STATE3           STATUS_LED_OFF
#define STATUS_LED_PERIOD3          (CONFIG_SYS_HZ / 2)

/* Silence the noise */
#define CONFIG_SYS_DEVICE_NULLDEV
#define CONFIG_SILENT_CONSOLE
#define CONFIG_SILENT_CONSOLE_UPDATE_ON_SET
#define CONFIG_SILENT_U_BOOT_ONLY

#endif /* __CONFIG_IPAFSM9XXX_H */
