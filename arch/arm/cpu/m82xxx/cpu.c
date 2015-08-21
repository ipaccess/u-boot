/*
 * Copyright(c) 2007-2014 Intel Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program; if not, write to the Free Software 
 * Foundation, Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 * The full GNU General Public License is included in this distribution 
 * in the file called LICENSE.GPL.
 *
 * Contact Information:
 * Intel Corporation
 */


#include <common.h>
#include <command.h>


#ifdef CONFIG_USE_IRQ
DECLARE_GLOBAL_DATA_PTR;
#endif

#ifdef CONFIG_L2_ON
void l2cache_disable(void);
#endif

#ifdef CONFIG_TURNER
// extern void disable_timer(void);
#endif

static void cache_flush(void);

/* read co-processor 15, register #1 (control register) */
static unsigned long read_p15_c1(void)
{
	unsigned long value;

	__asm__ __volatile__("mrc p15, 0, %0, c1, c0, 0\
			     @ read control reg\n":"=r"(value)
			     ::"memory");
	return value;
}

/* write to co-processor 15, register #1 (control register) */
static void write_p15_c1(unsigned long value)
{
	__asm__ __volatile__("mcr p15, 0, %0, c1, c0, 0\
			     @ write it back\n"::"r"(value)
			     : "memory");

	read_p15_c1();
}

static void cp_delay(void)
{
	/* Many OMAP regs need at least 2 nops */
	asm("nop");
	asm("nop");
}

/* See also ARM Ref. Man. */
#define C1_MMU		(1<<0)	/* mmu off/on */
#define C1_ALIGN	(1<<1)	/* alignment faults off/on */
#define C1_DC		(1<<2)	/* dcache off/on */
#define C1_WB		(1<<3)	/* merging write buffer on/off */
#define C1_BIG_ENDIAN	(1<<7)	/* big endian off/on */
#define C1_SYS_PROT	(1<<8)	/* system protection */
#define C1_ROM_PROT	(1<<9)	/* ROM protection */
#define C1_IC		(1<<12)	/* icache off/on */
#define C1_HIGH_VECTORS	(1<<13)	/* location of vectors: low/high addresses */
#define RESERVED_1	(0xf << 3)	/* must be 111b for R/W */

int cpu_init(void)
{
	/*
	 * setup up stacks if necessary
	 */
#ifdef CONFIG_USE_IRQ
	IRQ_STACK_START =
	    _armboot_start - CONFIG_SYS_MALLOC_LEN - CONFIG_SYS_GBL_DATA_SIZE - 4;
	FIQ_STACK_START = IRQ_STACK_START - CONFIG_STACKSIZE_IRQ;
#endif
	return 0;
}

int cleanup_before_linux(void)
{
	unsigned int i;

	/*
	 * this function is called just before we call linux
	 * it prepares the processor for linux
	 *
	 * we turn off caches etc ...
	 */
	disable_interrupts();

#ifdef CONFIG_TURNER
	// disable_timer();
#endif

	/* turn off I/D-cache */
	icache_disable();
	dcache_disable();

	/* invalidate I-cache */
	cache_flush();

#ifdef CONFIG_L2_ON
	/* turn off L2 cache */
	l2cache_disable();
	/* invalidate L2 cache also */
	v7_flush_dcache_all();
#endif
	i = 0;
	/* mem barrier to sync up things */
	asm("mcr p15, 0, %0, c7, c10, 4": :"r"(i));

#ifdef CONFIG_L2_ON
	l2cache_enable();
#endif

	return 0;
}

int do_reset(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	disable_interrupts();
	reset_cpu(0);

	/* NOTREACHED */
	return 0;
}

void icache_enable(void)
{
	ulong reg;

	reg = read_p15_c1();	/* get control reg. */
	cp_delay();
	write_p15_c1(reg | C1_IC);
}

void icache_disable(void)
{
	ulong reg;

	reg = read_p15_c1();
	cp_delay();
	write_p15_c1(reg & ~C1_IC);
}

int icache_status (void)
{
	return (read_p15_c1 () & C1_IC) != 0;
}


static void cache_flush(void)
{
	asm ("mcr p15, 0, %0, c7, c5, 0": :"r" (0));
}

void dcache_disable (void)
{
	ulong reg;

	reg = read_p15_c1 ();
	cp_delay ();
	write_p15_c1 (reg & ~C1_DC);
}

/* It makes no sense to use the dcache if the MMU is not enabled */
void dcache_enable (void)
{
	ulong reg;

	reg = read_p15_c1 ();
	cp_delay ();
	write_p15_c1 (reg | C1_DC);
}


int dcache_status (void)
{
	return (read_p15_c1 () & C1_DC) != 0;
}

void l2cache_enable(void)
{
	unsigned long i;

	__asm__ __volatile__("mrc p15, 0, %0, c1, c0, 1":"=r"(i));
	__asm__ __volatile__("orr %0, %0, #0x2":"=r"(i));
	__asm__ __volatile__("mcr p15, 0, %0, c1, c0, 1":"=r"(i));
}

void l2cache_disable(void)
{
	unsigned long i;

	__asm__ __volatile__("mrc p15, 0, %0, c1, c0, 1":"=r"(i));
	__asm__ __volatile__("bic %0, %0, #0x2":"=r"(i));
	__asm__ __volatile__("mcr p15, 0, %0, c1, c0, 1":"=r"(i));
}


