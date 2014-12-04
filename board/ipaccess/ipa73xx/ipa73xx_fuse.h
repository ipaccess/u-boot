/*
 * ipa73xx_fuse.h
 *
 *  Created on: 30 Nov 2014
 *      Author: rk6
 */

#ifndef IPA73XX_FUSE_H_
#define IPA73XX_FUSE_H_
#include <asm/arch-picoxcell/fuse.h>

void set_fuse_test_mode();
int read_fuse_in_range(unsigned int offset, char *buffer, unsigned int *buffer_len);
int blow_fuse_in_range(unsigned int offset,char *buffer,unsigned int *buffer_len);
#endif /* IPA73XX_FUSE_H_ */

