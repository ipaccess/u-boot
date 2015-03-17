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
 * Needed for UBI and command-line partitioning
 */
#define CONFIG_MTD_DEVICE
#define CONFIG_MTD_PARTITIONS
#define CONFIG_CMD_MTDPARTS

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

#endif
