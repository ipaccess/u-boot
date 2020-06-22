/*
 * IPAFSM90XX_fuse.h
 *
 *  Created on: 30 Nov 2014
 *      Author: rk6
 */

#ifndef IPAFSM90XX_FUSE_H_
#define IPAFSM90XX_FUSE_H_

#define QFPROM_FUSE_BASE_ADDRESS_RAW                 0xFC4B8000
#define QFPROM_FUSE_CTRL_REG_OFFSET                  0x204C
#define QFPROM_FUSE_CTRL_WRITE_BUSY                  (1 << 0)
#define QFPROM_CORR_FUSE_IMEI_ESN2                   QFPROM_FUSE_BASE_ADDRESS_RAW + 0x40E0
#define QFPROM_CORR_FUSE_FEC_EN                      QFPROM_FUSE_BASE_ADDRESS_RAW + 0x41D0
#define QFPROM_CORR_FUSE_PK_HASH_ROW0                QFPROM_FUSE_BASE_ADDRESS_RAW + 0x43D0
#define QFPROM_CORR_FUSE_PK_HASH_ROW1                QFPROM_FUSE_BASE_ADDRESS_RAW + 0x43D8
#define QFPROM_CORR_FUSE_PK_HASH_ROW2                QFPROM_FUSE_BASE_ADDRESS_RAW + 0x43E0
#define QFPROM_CORR_FUSE_PK_HASH_ROW3                QFPROM_FUSE_BASE_ADDRESS_RAW + 0x43E8
#define QFPROM_CORR_FUSE_SECURE_BOOT                 QFPROM_FUSE_BASE_ADDRESS_RAW + 0x43F8 


void set_fuse_test_mode(void);
int read_fuse_in_range(unsigned int offset, u32 *buffer, unsigned int *buffer_len);
int read_secure_boot_fuse(void);
uint8_t ipafsm90xx_is_board_fused(void);

#endif /* IPAFSM90XX_FUSE_H_ */

