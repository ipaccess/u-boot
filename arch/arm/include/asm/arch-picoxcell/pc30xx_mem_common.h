/*!
* \file pc30xx_mem_common.h
* \brief Definitions for the pc30xx memif_arm and memif-shared block.
*
* Copyright (c) 2010-2012 Picochip Ltd
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
* All enquiries to support@picochip.com
*/

#ifndef __PC30XX_MEM_COMMON_H__
#define __PC30XX_MEM_COMMON_H__

/*****************************************************************************/
/* PCTL Registers                                                            */
/*****************************************************************************/
#define PCTL_SCFG_OFFSET        (0x0000)
#define PCTL_SCTL_OFFSET        (0x0004)
#define PCTL_STAT_OFFSET        (0x0008)
#define PCTL_POWCTL_OFFSET      (0x0044)
#define PCTL_POWSTAT_OFFSET     (0x0048)
#define PCTL_CMDSTAT_OFFSET     (0x004C)
#define PCTL_CMDSTATEN_OFFSET   (0x0050)
#define PCTL_MCFG_OFFSET        (0x0080)
#define PCTL_DTUAWDT_OFFSET     (0x00B0)
#define PCTL_TOGCNT1U_OFFSET    (0x00C0)
#define PCTL_TINIT_OFFSET       (0x00C4)
#define PCTL_TRSTH_OFFSET       (0x00C8)
#define PCTL_TOGCNT100N_OFFSET  (0x00CC)

#define PCTL_TREFI_OFFSET       (0x00D0)
#define PCTL_TMRD_OFFSET        (0x00D4)
#define PCTL_TRFC_OFFSET        (0x00D8)
#define PCTL_TRP_OFFSET         (0x00DC)
#define PCTL_TRTW_OFFSET        (0x00E0)
#define PCTL_TAL_OFFSET         (0x00E4)
#define PCTL_TCL_OFFSET         (0x00E8)
#define PCTL_TCWL_OFFSET        (0x00EC)
#define PCTL_TRAS_OFFSET        (0x00F0)
#define PCTL_TRC_OFFSET         (0x00F4)
#define PCTL_TRCD_OFFSET        (0x00F8)
#define PCTL_TRRD_OFFSET        (0x00FC)
#define PCTL_TRTP_OFFSET        (0x0100)
#define PCTL_TWR_OFFSET         (0x0104)
#define PCTL_TWTR_OFFSET        (0x0108)
#define PCTL_TEXSR_OFFSET       (0x010C)
#define PCTL_TXP_OFFSET         (0x0110)
#define PCTL_TXPDLL_OFFSET      (0x0114)
#define PCTL_TZQCS_OFFSET       (0x0118)
#define PCTL_TZQCSI_OFFSET      (0x011C)
#define PCTL_TDQS_OFFSET        (0x0120)
#define PCTL_TCKSRE_OFFSET      (0x0124)
#define PCTL_TCKSRX_OFFSET      (0x0128)
#define PCTL_TCKE_OFFSET        (0x012C)
#define PCTL_TMOD_OFFSET        (0x0130)
#define PCTL_TRSTL_OFFSET       (0x0134)
#define PCTL_TZQCL_OFFSET       (0x0138)
#define PCTL_TMRR_OFFSET        (0x013C)
#define PCTL_TCKESR_OFFSET      (0x0140)
#define PCTL_TDPD_OFFSET        (0x0144)

#define PCTL_DFIPHYWRLAT_OFFSET (0x0254)
#define PCTL_DFITRDDATEN_OFFSET (0x0260)
#define PCTL_DFIPHYRDLAT_OFFSET (0x0264)
#define PCTL_DFISTSTAT0_OFFSET  (0x02C0)
#define PCTL_DFISTCFG0_OFFSET   (0x02C4)
#define PCTL_DFIUPDCFG_OFFSET   (0x0290)

#define PCTL_IPVR_OFFSET        (0x03F8)
#define PCTL_IPTR_OFFSET        (0x03FC)


/*****************************************************************************/
/* PUBL Registers                                                            */
/*****************************************************************************/
#define PUBL_RIDR_OFFSET        (0x0000)
#define PUBL_PIR_OFFSET         (0x0004)
#define PUBL_PGCR_OFFSET        (0x0008)
#define PUBL_PGSR_OFFSET        (0x000C)
#define PUBL_DLLGCR_OFFSET      (0x0010)
#define PUBL_ACDLLCR_OFFSET     (0x0014)
#define PUBL_PTR0_OFFSET        (0x0018)
#define PUBL_PTR1_OFFSET        (0x001C)
#define PUBL_PTR2_OFFSET        (0x0020)
#define PUBL_ACIOCR_OFFSET      (0x0024)
#define PUBL_DCR_OFFSET         (0x0030)
#define PUBL_DTPR0_OFFSET       (0x0034)
#define PUBL_DTPR1_OFFSET       (0x0038)
#define PUBL_DTPR2_OFFSET       (0x003C)
#define PUBL_MR0_OFFSET         (0x0040)
#define PUBL_MR1_OFFSET         (0x0044)
#define PUBL_MR2_OFFSET         (0x0048)
#define PUBL_MR3_OFFSET         (0x004C)

#define PUBL_BISTRR_OFFSET      (0x0100)
#define PUBL_BISTMSKR0_OFFSET   (0x0104)
#define PUBL_BISTMSKR1_OFFSET   (0x0108)
#define PUBL_BISTWCR_OFFSET     (0x010C)
#define PUBL_BISTLSR_OFFSET     (0x0110)
#define PUBL_BISTAR0_OFFSET     (0x0114)
#define PUBL_BISTAR1_OFFSET     (0x0118)
#define PUBL_BISTAR2_OFFSET     (0x011C)
#define PUBL_BISTUDPR_OFFSET    (0x0120)
#define PUBL_BISTGSR_OFFSET     (0x0124)
#define PUBL_BISTWER_OFFSET     (0x0128)
#define PUBL_BISTBER0_OFFSET    (0x012C)
#define PUBL_BISTBER1_OFFSET    (0x0130)
#define PUBL_BISTBER2_OFFSET    (0x0134)
#define PUBL_BISTWCSR_OFFSET    (0x0138)
#define PUBL_BISTFWR0_OFFSET    (0x013C)
#define PUBL_BISTFWR1_OFFSET    (0x0140)
#define PUBL_GPR0_OFFSET        (0x0178)
#define PUBL_GPR1_OFFSET        (0x017C)
#define PUBL_DX1DQTR_OFFSET     (0x0210)
#define PUBL_DX1DQSTR_OFFSET    (0x0214)

/*****************************************************************************/
/* MCTL Registers                                                            */
/*****************************************************************************/
#define MCTL_PCFG_0_OFFSET      (0x0400)
#define MCTL_PCFG_1_OFFSET      (0x0404)
#define MCTL_PCFG_2_OFFSET      (0x0408)
#define MCTL_PCFG_3_OFFSET      (0x040C)
#define MCTL_CCFG_OFFSET        (0x0480)
#define MCTL_DCFG_OFFSET        (0x0484)
#define MCTL_CSTAT_OFFSET       (0x048C)

#define MCTL_DCFG_IO_WIDTH_16BIT    (2)
#define MCTL_DCFG_IO_WIDTH_IDX      (0)
#define MCTL_DCFG_DRAM_DENSITY_2Gb  (5)
#define MCTL_DCFG_DRAM_DENSITY_1Gb  (4)
#define MCTL_DCFG_DRAM_DENSITY_IDX  (2)
#define MCTL_DCFG_DRAM_TYPE_14x9    (0)
#define MCTL_DCFG_DRAM_TYPE_13x10   (1)
#define MCTL_DCFG_DRAM_TYPE_IDX     (6)
#define MCTL_DCFG_ADDR_MAP_BRC      (0)
#define MCTL_DCFG_ADDR_MAP_RBC      (1)
#define MCTL_DCFG_ADDR_MAP_IDX      (8)
#define MCTL_PCFG_BP_RD_EN_IDX      (5)
#define MCTL_PCFG_BP_WR_EN_IDX      (4)

/* PIR Register bit fields */
#define PUBL_PIR_INIT           (1 << 0)
#define PUBL_PIR_DLLSRST        (1 << 1)
#define PUBL_PIR_DLLLOCK        (1 << 2)
#define PUBL_PIR_ZCAL           (1 << 3)
#define PUBL_PIR_ITMSRST        (1 << 4)
#define PUBL_PIR_DRAMRST        (1 << 5)
#define PUBL_PIR_DRAMINIT       (1 << 6)
#define PUBL_PIR_QSTRN          (1 << 7)
#define PUBL_PIR_EYETRN         (1 << 8)
#define PUBL_PIR_ICPC           (1 << 16)
#define PUBL_PIR_DLLBYP         (1 << 17)
#define PUBL_PIR_CTLDINIT       (1 << 18)
#define PUBL_PIR_CLRSR          (1 << 28)
#define PUBL_PIR_LOCKBYP        (1 << 29)
#define PUBL_PIR_ZCALBYP        (1 << 30)
#define PUBL_PIR_INITBYP        (1 << 31)

#endif /* __PC30XX_MEM_COMMON_H__ */
