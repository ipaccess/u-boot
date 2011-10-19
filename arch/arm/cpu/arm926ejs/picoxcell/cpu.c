/*****************************************************************************
 * $picoChipHeaderSubst$
 *****************************************************************************/

/*!
 * \file cpu.c
 * \brief Various useful functions.
 *
 * Copyright (c) 2006-2011 Picochip Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * All enquiries to support@picochip.com
 */

/* Includes ---------------------------------------------------------------- */
#include <common.h>
#include <asm/arch/utilities.h>
#include <netdev.h>

/* Macros ------------------------------------------------------------------ */

/* Constants --------------------------------------------------------------- */

/* Types ------------------------------------------------------------------- */

/* Prototypes--------------------------------------------------------------- */

/* Functions --------------------------------------------------------------- */

#if defined(CONFIG_DISPLAY_CPUINFO)
int print_cpuinfo (void)
{
	const char *part = picoxcell_get_partname ();
	unsigned int revision = picoxcell_get_revision ();

	printf ("CPU:   %s revision %u\n", part, revision);

	return 0;
}
#endif

int cpu_eth_init (bd_t *bis)
{
	int ret = -1;

#ifdef CONFIG_MACB
	ret = macb_eth_initialize (0, (void *)PICOXCELL_EMAC_BASE,
				   CONFIG_PHY_ADDR);
#endif

	return ret;
}

void enable_caches(void)
{
#ifndef CONFIG_SYS_ICACHE_OFF
	icache_enable();
#endif
}
