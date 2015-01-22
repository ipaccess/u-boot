/*
 * ipa73xx_fuse.h
 *
 *  Created on: 30 Nov 2014
 *      Author: rk6
 */

#ifndef IPA73XX_FUSE_H_
#define IPA73XX_FUSE_H_
#include <asm/arch-picoxcell/fuse.h>

#define PC73XX_KEY2_OFFSET 256
#define PC73XX_KEY2_SIZE 128
#define PC73XX_KEY2_RELEVANT_OFFSET 0
#define PC73XX_KEY2_RELEVANT_SIZE 113

#define PC73XX_KEY3_OFFSET 384
#define PC73XX_KEY3_SIZE 128
#define PC73XX_KEY3_RELEVANT_OFFSET 0
#define PC73XX_KEY3_RELEVANT_SIZE 113

#define PC73XX_KEY4_OFFSET 512
#define PC73XX_KEY4_SIZE 128
#define PC73XX_KEY4_RELEVANT_OFFSET 0
#define PC73XX_KEY4_RELEVANT_SIZE 96

void set_fuse_test_mode(void);
int read_fuse_in_range(unsigned int offset, char *buffer, unsigned int *buffer_len);
int blow_fuse_in_range(unsigned int offset,char *buffer,unsigned int *buffer_len);
int read_ethaddr_from_fuses(char* eth_addr_str, int index);
int read_secure_boot_fuse(void);
int read_production_mode_fuse(void);

#endif /* IPA73XX_FUSE_H_ */

