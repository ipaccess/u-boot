/*
 * Secure Monitor
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <config.h>
#include <version.h>
#include <asm/arch/hardware.h>

#define GIC_ICDDCR_BASE (A9IC_DISTR_BASEADDR + 0x000)
#define GIC_ICDISR_BASE (A9IC_DISTR_BASEADDR + 0x080)
#define GIC_ICDISE_BASE (A9IC_DISTR_BASEADDR + 0x100)
#define GIC_ICDICE_BASE (A9IC_DISTR_BASEADDR + 0x180)
#define GIC_ICDIPR_BASE (A9IC_DISTR_BASEADDR + 0x400)

#define GIC_ICCICR_BASE (A9IC_CPUINT_BASEADDR + 0x00)
#define GIC_ICCPMR_BASE (A9IC_CPUINT_BASEADDR + 0x04)

#define SCU_SNSAC_BASE (A9IC_SCU_BASEADDR + 0x54)

#define IRQ_PRIORITY_MASK (0xf0)

#define NSACR_CPN_MASK ((1<<13) - 1)
#define NSACR_D32_DIS (1<<14)
#define NSACR_ASE_DIS (1<<15)
#define NSACR_RFR (1<<19)
#define NSACR_TRC_DIS (1<<20)

#define CPACR_CPN_MASK ((1<<27) - 1)
#define CPACR_TRC_DIS (1<<28)
#define CPACR_TRC_DIS (1<<28)
#define CPACR_D32_DIS (1<<30)
#define CPACR_ASE_DIS (1<<31)

.globl normal_world
normal_world:

	stmfd sp!, {r0-r5, lr}

secondary_normal_world:

	/* disable GIC */
	ldr r0, =(GIC_ICDDCR_BASE)
	mov r1, #0x0
	str r1, [ r0, #0x00 ]

	/* GIC Interrupt Priority Mask Register:
	 *
	 * Only interrupts with higher priority than the value
	 * in this register can be signalled to the processor
	 *
	 * [7..0] - priority
	 **/
	ldr r0, =(GIC_ICCPMR_BASE)
	mov r1, #(IRQ_PRIORITY_MASK)
	str r1, [ r0, #0 ]

	/* GIC Interrupt Security Registers:
	 *
	 * for each bit:   0 - the corresponding interrupt is Secure
	 *                 1 - the corresponding interrupt is Non-secure
	 **/
	ldr r0, =(GIC_ICDISR_BASE)
	ldr r1, =0xffffffff
	str r1, [ r0, #0x04 ]
	str r1, [ r0, #0x08 ]
	str r1, [ r0, #0x0c ]
	str r1, [ r0, #0x10 ]
	str r1, [ r0, #0x14 ]
	str r1, [ r0, #0x18 ]
	str r1, [ r0, #0x1c ]
	str r1, [ r0, #0x20 ]

	ldr r0, =(GIC_ICDICE_BASE)
	ldr r1, =0xffff0000
	str r1, [ r0, #0x00 ]

	ldr r0, =(GIC_ICDISE_BASE)
	ldr r1, =0x0000ffff
	str r1, [ r0, #0x00 ]

	/* IPI and SGI */
	ldr r0, =(GIC_ICDISR_BASE)
	ldr r1, =0xffffffff
	str r1, [ r0, #0x00 ]

	ldr r0, =(GIC_ICDDCR_BASE)
	mov r1, #0x3
	str r1, [ r0, #0x00 ]

	ldr r0, =(GIC_ICCICR_BASE)
	mov r1, #0x7
	str r1, [ r0, #0x00 ]

	/* Non-secure Access Control Register:
	 *
	 * [18] - SMP bit
	 *                 0 - Non-secure write to ACTLR.SMP takes an exception
	 *                 1 - ACTLR.SMP can be modified in Non-secure mode
	 *
	 * [17] - lockable TLB in Normal World
	 *                 0 - lockable TLB can not be allocated
	 *                 1 - lockable TLB can be allocated
	 *
	 * [16] - Preload Engine
	 *                 0 - Only secure accesses to the CP15 c11 are permitted
	 *                 1 - Non-secure accesses to the CP15 c11 domain are permitted
	 *
	 * [13..0] - Non-secure access to coprocessor:
	 *                 0 - coprocessor can be accessed only from Secure state
	 *                 1 - coprocessor can be accessed from any security state
	 **/
	ldr r1, =(NSACR_CPN_MASK)
	orr r1, r1, #(1<<18)
	orr r1, r1, #(1<<17)
	orr r1, r1, #(1<<16)
	mcr p15, 0, r1, c1, c1, 2

	/* Coprocessor Access Control Register:
	 *
	 * [27..0] - Non-secure access to coprocessor:
	 *                 b00 - access denied
	 *                 b01 - access at PL1 only
	 *                 b10 - RESERVED
	 *                 b11 - full access
	 **/
	ldr r0, =(CPACR_CPN_MASK)
	mrc p15, 0, r1, c1, c0, 2
	orr r1, r1, r0

	mcr p15, 0, r1, c1, c0, 2

	/* SCU Non-secure Access Control Register:
	 *
	 **/
	ldr r0, =(SCU_SNSAC_BASE)
	ldr r1, =((1<<12) - 1)
	str r1, [ r0, #0 ]

	/* Secure Debug Enable Register:
	 *
	 **/
	mov r0, #0x3
	mcr p15, 0, r0, c1, c1, 1

	/* Secure Configuration Register:
	 *
	 * [0] - Non-secure bit
	 *                 0 - Secure state
	 *                 1 - Monitor mode: Secure state,
	 *                     All modes except Monitor: Non-secure state
	 **/
	mov r1, #0x1
	mcr p15, 0, r1, c1, c1, 0


	/* Welcome to Normal World! */


	/* Vector Base Address Register:
	 *
	 * Should be set explicitly in Normal mode
	 **/
	mov r1, #0         /* set to zero, so that SCTLR.V bit does matter */
	mcr p15, 0, r1, c12, c0, 0

	/* Multiprocessor Affinity Register:
	 *
	 * [1..0] - CPU ID
	 *                 0 - Processor is CPU0
	 *                 1 - Processor is CPU1
	 *                 2 - Processor is CPU2
	 *                 3 - Processor is CPU3
	 **/
	mrc p15, 0, r0, c0, c0, 5
	and r0, r0, #0x3
	cmp r0, #0
	beq master_normal_world

	/* CPU1: secondary entry, no stack available */
	mov r1, #0
        ldr r2, =(IRAM_BASEADDR)
        str r1, [ r2 ]

	b secondary_entry
	/* never returns */

/* CPU0 */
master_normal_world:
	/* setup second core */
	ldr r1, =secondary_normal_world
	ldr r2, =(IRAM_BASEADDR)
	str r1, [ r2 ]

	ldmfd sp!, {r0-r5, lr}
	mov pc, lr
