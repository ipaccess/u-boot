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

#include "ipa9131_fuse.h"
#include "secboot.h"
#include "led.h"

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_ML9131)
#define ML9131_VERSION 2014070000
#define ML9131_REVOCATION_VALUE 0

#define ml9131_macro_xstr(s) ml9131_macro_str(s)
#define ml9131_macro_str(s) #s

static const char CURRENT_ML9131_VERSION[] = "@(#)VERSION_VALUE=" ml9131_macro_xstr(ML9131_VERSION) "\0";
static const char CURRENT_ML9131_REVOCATION[] = "@(#)REVOCATION_VALUE=" ml9131_macro_xstr(ML9131_REVOCATION_VALUE) "\0";
#endif



int board_early_init_f(void)
{
	ccsr_gur_t *gur = (void *)CONFIG_SYS_MPC85xx_GUTS_ADDR;

	clrbits_be32(&gur->pmuxcr2, MPC85xx_PMUXCR2_UART_CTS_B0_GPIO42);
	setbits_be32(&gur->pmuxcr2, MPC85xx_PMUXCR2_UART_CTS_B0_DSP_TMS);

	clrbits_be32(&gur->pmuxcr2, MPC85xx_PMUXCR2_UART_RTS_B0_GPIO43);
	setbits_be32(&gur->pmuxcr2, MPC85xx_PMUXCR2_UART_RTS_B0_DSP_TCK |
			MPC85xx_PMUXCR2_UART_CTS_B1_SIM_PD |
			MPC85xx_PMUXCR2_ANT1_DIO4_7_GPIO85_88 |
			MPC85xx_PMUXCR2_ANT1_DIO8_9_GPIO21_22 |
			MPC85xx_PMUXCR2_ANT1_DIO10_11_GPIO23_24);
	setbits_be32(&gur->halt_req_mask, HALTED_TO_HALT_REQ_MASK_0);
	clrsetbits_be32(&gur->pmuxcr, MPC85xx_PMUXCR_IFC_AD_GPIO_MASK |
			MPC85xx_PMUXCR_IFC_AD17_GPO_MASK,
			MPC85xx_PMUXCR_IFC_AD_GPIO |
			MPC85xx_PMUXCR_IFC_AD17_GPO | MPC85xx_PMUXCR_SDHC_USIM);

	(void)ipa9131_fuse_init();

#if defined(CONFIG_ML9131)
	if (ML9131_REVOCATION_VALUE < ipa9131_fuse_read_loader_revocation()) {
		puts("Revoked\n");
		while (1) {
			setLED(LED_NWK, LED_RED);
			setLED(LED_SVC, LED_RED);
			setLED(LED_GPS, LED_RED);
			udelay(250000);
			setLED(LED_NWK, LED_YELLOW);
			setLED(LED_SVC, LED_YELLOW);
			setLED(LED_GPS, LED_YELLOW);
			udelay(250000);
		}
	}
#endif

	if (ipa9131_fuse_should_be_silent())
		gd->flags |= GD_FLG_SILENT;

	return 0;
}

/*****************************************************************************
 *
 * board_init()
 *
 * Purpose: Hardware platform initialisation functions
 *          (Actually for ipa9131 it's only for LED confidence function
 *
 * Returns: 0 - Success
 *
 *****************************************************************************/
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
#if defined(CONFIG_ML9131)
	led_confidence();
#endif
#if defined(CONFIG_CHARACTERISATION_IPA9131)
	if (1 != characterisation_init())
		 return 1;
#endif
	if (0 != load_security_requirements())
		return 1;

	if (silent_mode_enabled()) {
		setenv("silent", "1");
		setenv("bootdelay", "0");
	} else {
		setenv("silent", NULL);
	}

#if defined(CONFIG_CHARACTERISATION_IPA9131) && !defined(CONFIG_ML9131)
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
