/*
 * Copyright (c) 2015, ip.access Ltd.
 */
#ifndef SECURITY_H_20150415
#define SECURITY_H_20150415

#define DEVELOPMENT_MODE 0
#define TEST_MODE 1
#define PRODUCTION_MODE 2
#define SPECIALS_MODE 3

extern int read_operating_mode(unsigned int * oper_mode);
extern void read_board_revocation_count(unsigned int * rc);
extern int sec_is_trust_anchor(const unsigned char * pubkey_sha256_hash);

#endif
