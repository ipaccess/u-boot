/*
 * Copyright 2011-2012 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/fsl_law.h>
#include <asm/mmu.h>

struct law_entry law_table[] = {
	SET_LAW(CONFIG_SYS_NAND_BASE_PHYS, LAW_SIZE_1M, LAW_TRGT_IF_IFC),
	SET_LAW(CONFIG_SYS_FSL_DSP_CCSRBAR_PHYS, LAW_SIZE_1M,
		LAW_TRGT_IF_DSP_CCSR),
	SET_LAW(CONFIG_SYS_FSL_DSP_M2_RAM_ADDR, LAW_SIZE_16M,
		LAW_TRGT_IF_OCN_DSP),
#if !(defined(CONFIG_SPL_BUILD))
	/*Secure Memory Mapping to e500 address map*/
	SET_LAW(0xFED00000,LAW_SIZE_16K, 0x07),
#endif
};

int num_law_entries = ARRAY_SIZE(law_table);
