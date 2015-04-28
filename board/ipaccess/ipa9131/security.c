/*
 * Copyright (c) 2014, ip.access Ltd.
 */
#include <common.h>
#include <linux/string.h>

#include "security.h"

#include "ipa9131_fuse.h"
#include "hash.h"


#define REVOCATION_NUM_BITS 12




static const unsigned char production_trust_root_pubkey_hash[SHA256_HASH_BUFFER_SIZE] = {
    0xe0, 0x23, 0xac, 0x34, 0x05, 0xc9, 0xc4, 0x19, 0xa1, 0x48, 0x6d, 0xa2,
    0x84, 0x9f, 0xea, 0x9b, 0x2e, 0xb6, 0x3d, 0xe3, 0x10, 0x4f, 0x4b, 0x49,
    0xfb, 0x56, 0x5e, 0xea, 0x7e, 0x02, 0xf2, 0x1e
};

static const unsigned char test_trust_root_pubkey_hash[SHA256_HASH_BUFFER_SIZE] = {
    0xb5, 0x7a, 0x05, 0xa6, 0x8f, 0x09, 0x50, 0xde, 0xbd, 0x69, 0xf0, 0xc5,
    0x8b, 0x5b, 0xb1, 0xfd, 0x52, 0x49, 0xc3, 0x6c, 0x1e, 0xf4, 0xfe, 0x39,
    0xe8, 0x1d, 0xd3, 0x2e, 0x45, 0x7f, 0x2d, 0x6d
};

static const unsigned char development_trust_root_pubkey_hash[SHA256_HASH_BUFFER_SIZE] = {
    0x8d, 0xb0, 0x6a, 0x4c, 0x11, 0x03, 0x75, 0x9d, 0xcc, 0x17, 0xdf, 0xa2,
    0x9e, 0x14, 0x31, 0xb2, 0x46, 0x1b, 0xf4, 0x7d, 0x15, 0x08, 0x87, 0x59,
    0x35, 0x01, 0x3f, 0x38, 0x74, 0x99, 0x4c, 0x57
};





int sec_is_trust_anchor(const unsigned char * pubkey_sha256_hash)
{
    int ret;
    unsigned int opmode;

    ret = 0;

    if (0 == memcmp(pubkey_sha256_hash, production_trust_root_pubkey_hash, SHA256_HASH_BUFFER_SIZE))
    {
        ret = 1;
        goto cleanup;
    }

    if (0 != read_operating_mode(&opmode))
    {
        goto cleanup;
    }

    if ((opmode == SPECIALS_MODE || opmode == TEST_MODE || opmode == DEVELOPMENT_MODE) && (0 == memcmp(pubkey_sha256_hash, test_trust_root_pubkey_hash, SHA256_HASH_BUFFER_SIZE)))
    {
        ret = 1;
        goto cleanup;
    }

    if ((opmode == SPECIALS_MODE || opmode == DEVELOPMENT_MODE) && 0 == memcmp(pubkey_sha256_hash, development_trust_root_pubkey_hash, SHA256_HASH_BUFFER_SIZE))
    {
        ret = 1;
        goto cleanup;
    }

cleanup:
    return ret;
}




int read_operating_mode(unsigned int * oper_mode)
{
    u8 p;
    u8 d;
    u8 s;

    if (0 != ipa9131_fuse_read_security_profile(&p, &d, &s))
        return -1;

    if ((p && (d || s)) || (d && (p || s)) || (s && (p || d)))
    {
        *oper_mode = PRODUCTION_MODE;
        return 0;
    }

    if (p)
        *oper_mode = PRODUCTION_MODE;
    else if (d)
        *oper_mode = DEVELOPMENT_MODE;
    else if (s)
        *oper_mode = SPECIALS_MODE;
    else
        *oper_mode = DEVELOPMENT_MODE; /* TODO: perhaps later on this must return -1 */

    if (*oper_mode == DEVELOPMENT_MODE)
    {
        /* TODO: promote to test mode if appropriate */
    }
    
    return 0;
}




/**
 * Read the fused loader revocation count
 *
 * On failure, places the higest loader revocation count (12) into the output value.
 *
 * @param rc [OUT] The revocation count for this board.
 */
void read_board_revocation_count(unsigned int * rc)
{
    u16 r;

    *rc = REVOCATION_NUM_BITS;

    if (0 != ipa9131_fuse_read_loader_revocation(&r))
    {
        return;
    }

    *rc = r;
}
