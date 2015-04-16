/*
 * Copyright (c) 2015, ip.access Ltd.
 */
#ifndef HASH_H_20150415
#define HASH_H_20150415

#define HASH_ALGO_SHA1 0
#define HASH_ALGO_SHA256 1

#define SHA1_HASH_BUFFER_SIZE 20
#define SHA256_HASH_BUFFER_SIZE 32
#define HASH_BUFFER_SIZE SHA256_HASH_BUFFER_SIZE

extern int hash(const void * p, unsigned int len, unsigned int hash_algo, unsigned char * dst, unsigned int * dst_len);

#endif
