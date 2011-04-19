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
	unsigned int device_id, device_rev;

	printf ("CPU:   ");

	device_id = picoxcell_get_device_id ();	/* Read the device id */
	device_rev = picoxcell_get_revision ();	/* Read the revision code */

	switch (device_id) {
	case PC302_DEVICE_ID:
		printf ("PC302 Rev %01d\n", device_rev);
		break;
	case PC312_DEVICE_ID:
		printf ("PC312 Rev %01d\n", device_rev);
		break;
	case PC313_DEVICE_ID:
		printf ("PC313 Rev %01d\n", device_rev);
		break;
	case PC323_DEVICE_ID:
		printf ("PC323 Rev %01d\n", device_rev);
		break;
	case PC333_DEVICE_ID:
		printf ("PC333 Rev %01d\n", device_rev);
		break;
	case PC3008_DEVICE_ID:
		printf ("PC3008 Rev %01d\n", device_rev);
		break;
	default:
		panic ("Unknown device type !\n");
	}

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
