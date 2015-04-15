/*
 * Copyright (c) 2015, ip.access Ltd.
 */
#ifndef FLASH_H_20150415
#define FLASH_H_20150415

#include <common.h>
#include <nand.h>

static inline int flash_init(void) { return 0; }
static inline unsigned int flash_page_size(void) { return nand_info[0].writesize; }
static inline unsigned int flash_block_size(void) { return nand_info[0].erasesize / nand_info[0].writesize; }
static inline unsigned int flash_blocks(void) { return nand_info[0].size / nand_info[0].erasesize; }
static inline int flash_is_block_bad(unsigned int block) { return nand_block_isbad(&nand_info[0], block * nand_info[0].erasesize); }
extern int flash_read_page(unsigned int page, unsigned char * buffer);
extern int flash_read_bytes(unsigned char * dst, unsigned int src, unsigned int n);

#endif
