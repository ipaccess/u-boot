#ifndef IPAT2K_FUSE_H_20160421
#define IPAiT2K_FUSE_H_20160421

extern int ipat2k_fuse_init(void);
extern int ipat2k_is_unfused(void);
extern int ipat2k_fuses_are_write_protected(void);
extern int ipat2k_fuse_should_be_silent(void);
extern int ipat2k_fuse_read_eid(u64 * eid);
extern int ipat2k_fuse_read_security_profile(u8 * p, u8 * d, u8 * s);
extern int ipat2k_fuse_read_loader_revocation_value(u16 * r);
extern int ipat2k_fuse_read_application_revocation_value(u32 * r);
extern u16 ipat2k_fuse_read_loader_revocation(void);
extern u32 ipat2k_fuse_read_application_revocation(void);

#endif
