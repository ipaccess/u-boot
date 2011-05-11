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

/*****************************************************************************
 *
 * print_cpuinfo()
 *
 * Purpose: Display some useful info about the cpu we are running on.
 *
 * Returns: 0 - Success
 *
 *****************************************************************************/
int print_cpuinfo (void)
{
	const char *part = picoxcell_get_partname ();
	unsigned int revision = picoxcell_get_revision ();

	printf ("CPU:   %s revision %u\n", part, revision);

	return 0;
}

/*****************************************************************************
 *
 * cpu_eth_init()
 *
 * Purpose: Initialize the Ethernet controller
 *
 * Returns: 0 - Success
 *
 *****************************************************************************/
int cpu_eth_init (bd_t * bis)
{
	int ret = -1;

#ifdef CONFIG_DW_EMAC
	ret = picoxcell_eth_register (bis);
#endif

	return ret;
}
