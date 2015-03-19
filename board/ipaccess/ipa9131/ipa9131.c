/*
 * Copyright 2011-2012 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/processor.h>
#include <asm/mmu.h>
#include <asm/cache.h>
#include <asm/immap_85xx.h>
#include <asm/io.h>
#include <miiphy.h>
#include <libfdt.h>
#include <fdt_support.h>
#include <fsl_mdio.h>
#include <tsec.h>
#include <netdev.h>
#include <mtd_node.h>
#include <jffs2/load_kernel.h> /* sadly needed for the MTD FDT update */

#if defined(CONFIG_CHARACTERISATION_IPA9131)
#include "characterisation.h"
#endif


DECLARE_GLOBAL_DATA_PTR;

int board_early_init_f(void)
{
	ccsr_gur_t *gur = (void *)CONFIG_SYS_MPC85xx_GUTS_ADDR;

	clrbits_be32(&gur->pmuxcr2, MPC85xx_PMUXCR2_UART_CTS_B0_GPIO42);
	setbits_be32(&gur->pmuxcr2, MPC85xx_PMUXCR2_UART_CTS_B0_DSP_TMS);

	clrbits_be32(&gur->pmuxcr2, MPC85xx_PMUXCR2_UART_RTS_B0_GPIO43);
	setbits_be32(&gur->pmuxcr2, MPC85xx_PMUXCR2_UART_RTS_B0_DSP_TCK |
			MPC85xx_PMUXCR2_UART_CTS_B1_SIM_PD);
	setbits_be32(&gur->halt_req_mask, HALTED_TO_HALT_REQ_MASK_0);
	clrsetbits_be32(&gur->pmuxcr, MPC85xx_PMUXCR_IFC_AD_GPIO_MASK |
			MPC85xx_PMUXCR_IFC_AD17_GPO_MASK,
			MPC85xx_PMUXCR_IFC_AD_GPIO |
			MPC85xx_PMUXCR_IFC_AD17_GPO | MPC85xx_PMUXCR_SDHC_USIM);

	return 0;
}

int checkboard(void)
{
#if !defined(CONFIG_MISC_INIT_R) || !defined(CONFIG_CHARACTERISATION_IPA9131)
	struct cpu_type *cpu;

	cpu = gd->arch.cpu;
	printf("Board: %sRDB\n", cpu->name);
#endif
	return 0;
}

#if defined(CONFIG_MISC_INIT_R)
int misc_init_r(void)
{
#if defined(CONFIG_CHARACTERISATION_IPA9131)
	int ret;

	ret = characterisation_init();

	if (ret != 1)
		return 1;

	(void)print_characterisation();
#endif
	return 0;
}
#endif

#if defined(CONFIG_OF_BOARD_SETUP)
void ft_board_setup(void *blob, bd_t *bd)
{
	phys_addr_t base;
	phys_size_t size;

	struct node_info nodes[] = {
		{ "fsl,espi-flash",	MTD_DEV_TYPE_NOR },  /* SPI-NOR flash */
		{ "fsl,ifc-nand",	MTD_DEV_TYPE_NAND }, /* NAND flash */
	};

	ft_cpu_setup(blob, bd);

	base = getenv_bootm_low();
	size = getenv_bootm_size();

	fdt_fixup_memory(blob, (u64)base, (u64)size);

	fdt_fixup_dr_usb(blob, bd);

	puts("Updating MTD partitions...\n");
	fdt_fixup_mtdparts(blob, nodes, ARRAY_SIZE(nodes));
}
#endif
