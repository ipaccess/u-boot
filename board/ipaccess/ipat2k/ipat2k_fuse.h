#ifndef IPAT2K_FUSE_H_20160421
#define IPAT2K_FUSE_H_20160421


#include "efuse.h"

#define INTEL_DEFINED_EFUSE_INSTANCE            0
#define READ_ONCE_PER_BOOT_EFUSE_INSTANCE       1
#define LAST_TIME_PROG_EFUSE_INSTANCE           2
#define DIS_EXTERNAL_PIN_EFUSE_INSTANCE         3
#define OEM_SIGN_KEY_HASH_PART_1_INSTANCE       4
#define OEM_SIGN_KEY_HASH_PART_2_INSTANCE       5
#define AP_CHARACTERISATION_EFUSE_INSTANCE      6
#define REVOCATION_COUNT_EFUSE_INSTANCE         7
#define MAC_ADDR_EFUSE_INSTANCE                 8
#define MASTER_KEY_EFUSE_INSTANCE_START         9
#define MASTER_KEY_EFUSE_INSTANCE_END           10

extern uint8_t ipat2k_is_secure_boot();
extern uint8_t ipat2k_is_board_fused();
uint8_t ipat2k_fuse_read_loader_revocation();
uint8_t ipat2k_fuse_read_application_revocation();

#endif
