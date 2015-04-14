/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * CPU specific code
 */

#include <common.h>
#include <command.h>
#include <asm/system.h>

static void cache_flush(void);

int cleanup_before_linux (void)
{
	/*
	 * this function is called just before we call linux
	 * it prepares the processor for linux
	 *
	 * we turn off caches etc ...
	 */

	disable_interrupts ();


	/* turn off I/D-cache */
	icache_disable();
	dcache_disable();
	l2_cache_disable();

	/* flush I/D-cache */
	cache_flush();

	return 0;
}

/* flush I/D-cache */
static void cache_flush (void)
{
	unsigned long i = 0;

    // For more info on cache flushing see ARM doc "3.2.22. c7, Cache operations":
    // http://infocenter.arm.com/help/topic/com.arm.doc.ddi0333h/Babhejba.html
    asm ("mcr	p15, 0, r0, c7, c14, 0": :"r"(i));  // clean+invalidate D cache
    asm ("mcr	p15, 0, r0, c7, c5, 0": :"r"(i));   // invalidate I cache
    asm ("mcr	p15, 0, r0, c7, c5, 6": :"r"(i));   // Flush Entire Branch Target Cache
}
