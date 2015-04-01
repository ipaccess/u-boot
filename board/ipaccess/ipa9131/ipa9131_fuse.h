#ifndef IPA9131_FUSE_H_20150401
#define IPA9131_FUSE_H_20150401

extern int ipa9131_fuse_init(void);
extern int ipa9131_fuse_read_eid(u64 * eid);
extern int ipa9131_fuse_read_security_profile(u8 * p, u8 * d, u8 * s);
extern int ipa9131_fuse_read_loader_revocation(u16 * r);

#endif
