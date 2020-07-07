#ifndef __CONFIG_IPAFSM90XX_H
#define __CONFIG_IPAFSM90XX_H

#include <linux/sizes.h>
#include <asm/mach-types.h>

/* #define DEBUG */

#define CONFIG_MACH_TYPE MACH_TYPE_FSM9XXX_SURF
#define CONFIG_IPAFSM90XX

/* SDRAM configuration */
#define CONFIG_SYS_SDRAM_BASE (0x00000000)
#define CONFIG_SYS_SDRAM_SIZE 0x40000000
#define CONFIG_NR_DRAM_BANKS (1)
#define PHYS_SDRAM_1        0x00000000  /* SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE   0x40000000  /* 1 GB */
#define CONFIG_STACKSIZE    (128*1024)

#define CONFIG_BITREVERSE
#define CONFIG_SYS_GENERIC_BOARD
#define CONFIG_SYS_BAUDRATE_TABLE { 4800, 9600, 19200, 38400, 57600, 115200 }
#define CONFIG_BAUDRATE 115200

#define CONFIG_SYS_MALLOC_LEN SZ_16M
#define CONFIG_SYS_BOOTM_LEN SZ_32M

#define CONFIG_SYS_TEXT_BASE 0x18a00000

#define CONFIG_SYS_INIT_SP_ADDR (CONFIG_SYS_TEXT_BASE - (3*SZ_4M)) /* stack is at ???, growing down and with global data above */

#define CONFIG_SYS_MAXARGS (16)
#define CONFIG_SYS_CBSIZE SZ_1K
#define CONFIG_SYS_BARGSIZE CONFIG_SYS_CBSIZE
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT_HUSH_PS2 "ipafsm90xx> "


#define CONFIG_SYS_LOAD_ADDR    0x02000000
#define CONFIG_LOADADDR     0x02000000

#define CONFIG_SYS_HZ_CLOCK 19200000 
#define CONFIG_SYS_HZ 1000

#define CONFIG_MSM_SERIAL 1
#define CONFIG_CONS_INDEX 1

#define CONFIG_MISC_INIT_R
#define CONFIG_SKIP_LOWLEVEL_INIT

#include "config_cmd_default.h"

#define CONFIG_SYS_PROMPT   "ipafsm90xx> "
#define CONFIG_BOOTCOUNT_IPAFSM90XX

/* Auto-boot options */
#define CONFIG_BOOTDELAY 3
#define CONFIG_AUTOBOOT_KEYED
#define CONFIG_AUTOBOOT_STOP_STR "stop"
#define CONFIG_AUTOBOOT_PROMPT "autoboot in %d seconds (\"stop\" to stop)\n",bootdelay

 
/* Use 'long' help messages */
#define CONFIG_SYS_LONGHELP 

#define CONFIG_SYS_NO_FLASH
//TODO correct flash config (remove the following hacks to get it compiling)
//#define CONFIG_SYS_NO_FLASH   with this cfi_flash.h is broken
#define CONFIG_SYS_MAX_FLASH_SECT 4096
#define CONFIG_SYS_MAX_FLASH_BANKS  1
#undef CONFIG_CMD_IMLS


#define CONFIG_CMD_MD5SUM
#define CONFIG_MD5


/* Enable FIT and OF control support */
#define CONFIG_OF_CONTROL
#define CONFIG_OF_EMBED
#define CONFIG_DEFAULT_DEVICE_TREE "ipafsm90xx"
#define CONFIG_OF_LIBFDT
#define CONFIG_FIT
#define CONFIG_FIT_SIGNATURE
#define CONFIG_FIT_SIGNATURE_REQUIRED_KEYNODE_ONLY
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


#define CONFIG_BOOTCOUNT_IPAFSM90XX
#define CONFIG_BOOTCOUNT_LIMIT
#define CONFIG_BOOTCOUNT_LIMIT_COUNT 4
#define CONFIG_BOOTCOUNT_FSM90XX_MODULUS (CONFIG_BOOTCOUNT_LIMIT_COUNT*2)


/*MMC defines*/
#define CONFIG_GENERIC_MMC
#define CONFIG_CMD_MMC
#define CONFIG_MMC
#define CONFIG_MSM_MMC
//#define CONFIG_MMC_TRACE

#define CONFIG_FS_EXT4
#define CONFIG_CMD_EXT4
#define CONFIG_EFI_PARTITION
#define CONFIG_EXT4_WRITE
#define CONFIG_CMD_EXT4_WRITE

//MSM mmc defines 
//#define CONFIG_SYS_MMC_MAX_BLK_COUNT    0x800 //read 1 MB at a time from adma.

//IPA configs
#define CONFIG_CMD_KEY
#define CONFIG_CMD_LEDC
#define CONFIG_CMD_SECPARM
#define CONFIG_CMD_LIE


#define IPA_BASE_BOOTARGS                               \
    " earlyprintk"                                     \
    " maxcpus=4"                                        \
    " coherent_pool=3M"                                 \
    " mem=196M@0x03c00000"                              \
    " mem=412M@0x26400000 vmalloc=512M"                 \
    " androidboot.emmc=true androidboot.baseband=msm"


#define LINUX_CONSOLEDEV "ttyHSL0"
#define CMDLINE_ARGS_LINUX IPA_BASE_BOOTARGS " console=" LINUX_CONSOLEDEV ","  __stringify(CONFIG_BAUDRATE) " elevator=noop"
#define CMDLINE_ARGS_LINUX_SILENT IPA_BASE_BOOTARGS " console=tty0 " " quiet"

#define SET_BOOTARGS                                   \
    "if silent_mode_enabled; then "                    \
    " setenv bootargs " CMDLINE_ARGS_LINUX_SILENT "; " \
    "else; "                                           \
    " setenv bootargs " CMDLINE_ARGS_LINUX "; "        \
    "fi; "

#define SECURE_BOOT_COMMAND                         \
    "NO_SEC_OK=0; "                             \
    "DEV_KEY_OK=0; "                                \
    "TST_KEY_OK=0; "                                \
    "PRD_KEY_OK=0; "                                \
    "secparm blank; "                               \
    "if test $? -eq 0; then "                           \
     "NO_SEC_OK=1; "                                \
    "fi; "                                  \
    "secparm spcmode; "                             \
    "if test $? -eq 0; then "                           \
     "NO_SEC_OK=1; "                                \
    "fi; "                                  \
    "secparm devmode; "                             \
    "if test $? -eq 0; then "                           \
     "DEV_KEY_OK=1; "                               \
     "TST_KEY_OK=1; "                               \
     "PRD_KEY_OK=1; "                               \
    "fi; "                                  \
    "secparm tstmode; "                             \
    "if test $? -eq 0; then "                           \
     "TST_KEY_OK=1; "                               \
     "PRD_KEY_OK=1; "                               \
    "fi; "                                  \
    "secparm prdmode; "                             \
    "if test $? -eq 0; then "                           \
     "PRD_KEY_OK=1; "                               \
    "fi; "                                  \
    "if test $PRD_KEY_OK -eq 1; then "                      \
     "if key unrequire dev; then "                      \
      "if key unrequire tstoem0; then "                     \
       "if key require ipaoem0 conf; then "                 \
        "if bootm ${loadaddr}#${selected_config}; then "            \
        "ledc all green red 3 1000;"                        \
         "reset; "                              \
        "fi; "                                  \
       "fi; "                                   \
      "fi; "                                    \
     "fi; "                                 \
    "fi; "                                  \
    "if test $TST_KEY_OK -eq 1; then "                      \
     "if key unrequire dev; then "                      \
      "if key unrequire ipaoem0; then "                     \
       "if key require tstoem0 conf; then "                 \
        "if bootm ${loadaddr}#${selected_config}; then "            \
        "ledc all green red 3 1000;"                        \
         "reset; "                              \
        "fi; "                                  \
       "fi; "                                   \
      "fi; "                                    \
     "fi; "                                 \
    "fi; "                                  \
    "if test $DEV_KEY_OK -eq 1; then "                      \
     "if key unrequire tstoem0; then "                      \
      "if key unrequire ipaoem0; then "                     \
       "if key require dev conf; then "                     \
        "if bootm ${loadaddr}#${selected_config}; then "            \
        "ledc all green red 3 1000;"                        \
         "reset; "                              \
        "fi; "                                  \
       "fi; "                                   \
      "fi; "                                    \
     "fi; "                                 \
    "fi; "                                  \
    "if test $NO_SEC_OK -eq 1; then "                       \
     "if key unrequire tstoem0; then "                      \
      "if key unrequire ipaoem0; then "                     \
       "if key unrequire dev; then "                        \
        "if bootm ${loadaddr}#${selected_config}; then "            \
        "ledc all green red 3 1000;"                        \
         "reset; "                              \
        "fi; "                                  \
       "fi; "                                   \
      "fi; "                                    \
     "fi; "                                 \
    "fi; "                                  \
    "ledc all green red 3 1000;"                        \
    "reset"

#define STANDARD_BOOT_COMMAND                                                       \
    "ledc nwk svc green off 1 300; "                                                \
    "run select_bootargs; "                                                         \
    "run select_config; "                                                           \
    "setenv fsactive fs1; "                                                         \
    "setenv fsstandby fs0; "                                                        \
    "if test -e mmc 0:$fs1_pnum primary.flag; then "                                \
    "  if ext4load mmc 0:$fs1_pnum $loadaddr fitImage; then "                       \
    "     setenv bootargs $bootargs fsactive=$fsactive fsstandby=$fsstandby; "      \
    "     run secureboot; "                                                         \
    "  fi; "                                                                        \
    "fi; "                                                                          \
    "setenv fsactive fs0; "                                                         \
    "setenv fsstandby fs1; "                                                        \
    "if ext4load mmc 0:$fs0_pnum $loadaddr fitImage; then "                         \
    "  setenv bootargs $bootargs fsactive=$fsactive fsstandby=$fsstandby; "         \
    "  run secureboot; "                                                            \
    "fi; "                                                                          \
    "ledc all green red 3 1000;"                                                    \
    "reset"

#define FALLBACK_BOOT_COMMAND                                                       \
    "ledc nwk svc green off 1 300; "                                                \
    "run select_bootargs; "                                                         \
    "run select_config; "                                                           \
    "setenv fsactive fs1; "                                                         \
    "setenv fsstandby fs0; "                                                        \
    "if test -e mmc 0:$fs1_pnum primary.flag; then "                                \
    "  setenv fsactive fs0; "                                                       \
    "  setenv fsstandby fs1; "                                                      \
    "  if ext4load mmc 0:$fs0_pnum $loadaddr fitImage; then "                       \
    "     setenv bootargs $bootargs fsactive=$fsactive fsstandby=$fsstandby; "      \
    "     run secureboot; "                                                         \
    "  fi; "                                                                        \
    "else; "                                                                        \
    "  if ext4load mmc 0:$fs1_pnum $loadaddr fitImage; then "                       \
    "    setenv bootargs $bootargs fsactive=$fsactive fsstandby=$fsstandby; "       \
    "    run secureboot; "                                                          \
    "  fi; "                                                                        \
    "fi; "                                                                          \
    "ledc all green red 3 1000;"                                                    \
    "reset"


/*
 * This command block is responsible for selecting the correct FDT to pass to
 * Linux based on the characterised board type.  The "current" state of the art
 * is the default, and older FDTs are selected based on the board variant part
 * number (503, 509 etc.)
 *
 * This sets up the 'selected_config' environment variable, which is used by
 * the secure boot command to pass as a config selector to the bootm command.
 */
#define SELECT_CONFIG                                       \
    "setenv selected_config config@2; "                     \
    "if test -n \"${board_variant_part}\"; then "           \
     "if test ${board_variant_part} -eq 503; then "         \
      "setenv selected_config config@1; "                   \
     "fi; "                                                 \
    "fi"


#define CONFIG_EXTRA_ENV_SETTINGS                               \
    "autoload=no\0"                                             \
    "autostart=no\0"                                            \
    "fdt_high=0x5C0A000\0"                                      \
    "initrd_high=0x82d3000\0"                                   \
    "select_bootargs=" SET_BOOTARGS "\0"                        \
    "select_config=" SELECT_CONFIG "\0"                         \
    "altbootcmd=" FALLBACK_BOOT_COMMAND "\0"                    \
    "bootlimit=" __stringify(CONFIG_BOOTCOUNT_LIMIT_COUNT) "\0" \
    "secureboot=" SECURE_BOOT_COMMAND "\0"


#define CONFIG_BOOTCOMMAND STANDARD_BOOT_COMMAND
#define CONFIG_SILENT_CONSOLE
#define CONFIG_SILENT_CONSOLE_UPDATE_ON_SET
#define CONFIG_SILENT_CONSOLE_UPDATE_ON_RELOC
/*Silent u-boot only with CONFIG_SILENT_CONSOLE, for kernel seprate bootargs environment variable is set*/
#define CONFIG_SILENT_U_BOOT_ONLY


/* MMC ENV related defines */
#undef CONFIG_ENV_OFFSET
#undef CONFIG_ENV_SIZE

#define CONFIG_ENV_IS_IN_MMC
#define CONFIG_SYS_MMC_ENV_DEV      0       /* SLOT2: eMMC(1) */
//#define CONFIG_SYS_MMC_ENV_PART     0
#define CONFIG_ENV_OFFSET           (0x0027ad82 * 512)     /* (in bytes) 768 KB */
#define CONFIG_ENV_SIZE             SZ_256K  /*256 KB*/
#define CONFIG_ENV_SIZE_REDUND      CONFIG_ENV_SIZE
#define CONFIG_ENV_OFFSET_REDUND    (0x0027b182 * 512)

#define CONFIG_SYS_REDUNDAND_ENVIRONMENT
#define CONFIG_CMD_SAVEENV


//board characterisation
#define CONFIG_CHARACTERISATION
#define CONFIG_CHARACTERISATION_SIZE 256 //in bytes
#define CONFIG_CHARACTERISATION_MMC_PART_NAME "chr"

#define CONFIG_CMD_LIE

#define CONFIG_HW_WATCHDOG
//debugging configs: keep disabled

//#define CONFIG_IMAGE_FORMAT_LEGACY
//#define DEBUG_BOOTKEYS 1
/* Allow us to track boot progress */
//#define CONFIG_SHOW_BOOT_PROGRESS  1
#endif /* __CONFIG_IPAFSM90XX_H */
