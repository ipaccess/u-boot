/*
 * Copyright (c) 2015, ip.access Ltd.
 */
#include "hash.h"

#include <common.h>
#include <u-boot/sha1.h>
#include <u-boot/sha256.h>




int hash(const void * p, unsigned int len, unsigned int hash_algo, unsigned char * dst, unsigned int * dst_len)
{
    if (!p || !len || !dst || !dst_len || !*dst_len)
    {
        return -1;
    }

#if defined(CONFIG_SHA1)
    if (HASH_ALGO_SHA1 == hash_algo)
    {
        if (*dst_len < SHA1_HASH_BUFFER_SIZE)
        {
            return -1;
        }

        sha1_csum((const unsigned char *)p, len, dst);
        *dst_len = SHA1_HASH_BUFFER_SIZE;
        return 0;
    }
#endif
#if defined(CONFIG_SHA256)
    if (HASH_ALGO_SHA256 == hash_algo)
    {
        if (*dst_len < SHA256_HASH_BUFFER_SIZE)
        {
            return -1;
        }

        sha256_csum_wd((const unsigned char *)p, len, dst, CHUNKSZ_SHA256);
        *dst_len = SHA256_HASH_BUFFER_SIZE;
        return 0;
    }
#endif
    return -1;
}
