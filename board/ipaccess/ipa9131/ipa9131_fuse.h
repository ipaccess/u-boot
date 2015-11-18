#ifndef IPA9131_FUSE_H_20150401
#define IPA9131_FUSE_H_20150401

#define IPA9131_LOADER_REVOCATION_MAX 12
#define IPA9131_APPLICATION_REVOCATION_MAX 28
#define IPA9131_MINIMAL_OTPMK_VALUE 0x0F
#define IPA9131_MINIMAL_JTAG_RESP_VALUE 0x78000000

#define SFP_INGR_ADDRESS        (CONFIG_SYS_IMMR + 0x000e7020)
#define SFP_DESSR_ADDRESS       (CONFIG_SYS_IMMR + 0x000e7024)
#define SFP_SFPCR_ADDRESS       (CONFIG_SYS_IMMR + 0x000e7028)
#define SFP_OSPR_ADDRESS        (CONFIG_SYS_IMMR + 0x000e7040)
#define SFP_FSWPR_ADDRESS       (CONFIG_SYS_IMMR + 0x000e7044)
#define SFP_DPR_ADDRESS         (CONFIG_SYS_IMMR + 0x000e7048)
#define SFP_DCVR0_ADDRESS       (CONFIG_SYS_IMMR + 0x000e704c)
#define SFP_DCVR1_ADDRESS       (CONFIG_SYS_IMMR + 0x000e7050)
#define SFP_DRVR0_ADDRESS       (CONFIG_SYS_IMMR + 0x000e7054)
#define SFP_DRVR1_ADDRESS       (CONFIG_SYS_IMMR + 0x000e7058)
#define SFP_OTPMKR0_ADDRESS     (CONFIG_SYS_IMMR + 0x000e705c)
#define SFP_OTPMKR1_ADDRESS     (CONFIG_SYS_IMMR + 0x000e7060)
#define SFP_OTPMKR2_ADDRESS     (CONFIG_SYS_IMMR + 0x000e7064)
#define SFP_OTPMKR3_ADDRESS     (CONFIG_SYS_IMMR + 0x000e7068)
#define SFP_OTPMKR4_ADDRESS     (CONFIG_SYS_IMMR + 0x000e706c)
#define SFP_OTPMKR5_ADDRESS     (CONFIG_SYS_IMMR + 0x000e7070)
#define SFP_OTPMKR6_ADDRESS     (CONFIG_SYS_IMMR + 0x000e7074)
#define SFP_OTPMKR7_ADDRESS     (CONFIG_SYS_IMMR + 0x000e7078)
#define SFP_SRKHR0_ADDRESS      (CONFIG_SYS_IMMR + 0x000e707c)
#define SFP_SRKHR1_ADDRESS      (CONFIG_SYS_IMMR + 0x000e7080)
#define SFP_SRKHR2_ADDRESS      (CONFIG_SYS_IMMR + 0x000e7084)
#define SFP_SRKHR3_ADDRESS      (CONFIG_SYS_IMMR + 0x000e7088)
#define SFP_SRKHR4_ADDRESS      (CONFIG_SYS_IMMR + 0x000e708c)
#define SFP_SRKHR5_ADDRESS      (CONFIG_SYS_IMMR + 0x000e7090)
#define SFP_SRKHR6_ADDRESS      (CONFIG_SYS_IMMR + 0x000e7094)
#define SFP_SRKHR7_ADDRESS      (CONFIG_SYS_IMMR + 0x000e7098)
#define SFP_OUIDR_ADDRESS       (CONFIG_SYS_IMMR + 0x000e709c)
#define SFP_OVPR_ADDRESS        (CONFIG_SYS_IMMR + 0x000e70a4)
#define SFP_OSCR_ADDRESS        (CONFIG_SYS_IMMR + 0x000e70a8)
#define SFP_ONSEC_ADDRESS       (CONFIG_SYS_IMMR + 0x000e70ac)
#define SFP_FUIDR_ADDRESS       (CONFIG_SYS_IMMR + 0x000e70b0)
#define SFP_FSCR_ADDRESS        (CONFIG_SYS_IMMR + 0x000e70b8)
#define SFP_IBRConfig_ADDRESS   (CONFIG_SYS_IMMR + 0x000e70bc)

#define SFP_LOWEST_ADDRESS      SFP_DESSR_ADDRESS
#define SFP_HIGHEST_ADDRESS     SFP_IBRConfig_ADDRESS


extern int ipa9131_fuse_init(void);
extern int ipa9131_is_unfused(void);
extern int ipa9131_fuses_are_write_protected(void);
extern int ipa9131_fuse_should_be_silent(void);
extern int ipa9131_fuse_read_eid(u8 oui_arr[3],u32 *serial);
extern int ipa9131_fuse_read_security_profile(u8 * p, u8 * d, u8 * s);
extern int ipa9131_fuse_read_loader_revocation_value(u16 * r);
extern int ipa9131_fuse_read_application_revocation_value(u32 * r);
extern u16 ipa9131_fuse_read_loader_revocation(void);
extern u32 ipa9131_fuse_read_application_revocation(void);
extern u32 ipa9131_fuse_read_fsl_uid(void);
extern void ipa9131_blow_fuse(void);
extern int ipa9131_fuse_write_in_range(u32 start_addr, u8 num_words, const u32* val);
extern void ipa9131_fuse_read_in_range(u32 start_addr, u8 num_words, u32* val);
extern int ipa9131_read_provisioning_status(u8 *otpmk_set,u8 *dbg_resp_set, u8 *apk_created );
extern int ipa9131_fuse_its_blown(void);
extern void ipa9131_fuse_enable_blowing(void);
extern void ipa9131_fuse_disable_blowing(void);
#endif
