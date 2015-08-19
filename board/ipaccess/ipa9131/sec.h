#ifndef IPA9131_SEC_H_20150401
#define IPA9131_SEC_H_20150401

#define sec_out_be32(x,y) out_be32((volatile unsigned __iomem *)(x),(y))
#define sec_in_be32(x) in_be32((const volatile unsigned __iomem *)(x))

#define SECMON_HPCOMR        (CONFIG_SYS_IMMR + 0x000e6004)
#define SECMON_HPSR          (CONFIG_SYS_IMMR + 0x000e6014)
#define SECMON_HPSVSR        (CONFIG_SYS_IMMR + 0x000e6018)

#endif
