/*
 * Copyright (c) 2014, ip.access Ltd.
 */
#ifndef IMAGE_H_20140923
#define IMAGE_H_20140923

#include "fimage.h"

#define APPS9131_SEARCH_START_BLOCK 8
#define APPS9131_SEARCH_NUM_BLOCKS 16
#define APPS9131_SEARCH_IMAGE_TYPE FIMAGE_HEADER_TYPE_APPS9131
#define APPS9131_MAX_IMAGE_SIZE 0x00100000
#define APPS9131_IMAGE_ENTRY_POINT 0x00200000

extern int load_image(unsigned int start_block, unsigned int num_blocks, unsigned int image_type);

#endif
