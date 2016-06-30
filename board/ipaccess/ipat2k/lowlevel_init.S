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

#include <config.h>
#include <version.h>
#include <asm/arch/hardware.h>


_TEXT_BASE:
    .word CONFIG_SYS_TEXT_BASE
    
/* Set up the platform, once the cpu has been initialized */
.globl lowlevel_init
lowlevel_init:

	/* Check if we are operating from DDR */
	mov r0,pc
	ldr r1,_TEXT_BASE
	cmp r0,r1
	bge exit

	/* relocate the piece of code configuring boot CS #0 to IRAM */
relocate_remap:				/* relocate U-Boot to RAM	    */
	adr	r0, flash_remap		/* pc relative  address of label    */
	ldr	r1, =0xf4000020		/* linked image address of label    */
	adr	r2, flash_remap
	adr	r3, flash_remap_end
	sub	r2, r3, r2		/* r2 <- size of armboot            */
	add	r2, r0, r2		/* r2 <- source end address         */

copy_loop:
	ldmia	r0!, {r3-r10}		/* copy from source address [r0]    */
	stmia	r1!, {r3-r10}		/* copy to   target address [r1]    */
	cmp	r0, r2			/* until source end addreee [r2]    */
	ble	copy_loop
	
	/* Start from IRAM to configure CS0 */	
	adr	r3, run_from_flash	@ keep program offset to start on flash
	ldr	r0, =0xf4000020
	mov 	pc, r0

flash_remap:		    
	mov r0,#0xf4000000			
	orr r0,r0,#0x00c00000
	orr r0,r0,#0x00008000		@ load expansion bus base address
	ldr r1,[r0]			@ load general control register
	bic r1,r1,#0x1000000		@ clean remap bit, so ddr can start from 0
	str r1,[r0]			@ store general control register
	dsb
	
	mov r0,#0
	add r0,r0,#0xfe100000		@ expansion bus base address
	mov r1,#1
	str r1,[r0]			@ SW reset

        mov r1,#0x10000			@ Small delay after EXP reset
_delay_0:
        subs r1,r1,#1
        bne _delay_0
	
	mov r1,#0xA0000			@ flash base >> 12
	str r1,[r0,#0x08]		@ write CS0 base register
	dsb
	orr r1,#0x0ff
	orr r1,r1,#0x7f00		@ (flash base >> 12) | 0x7fff
	str r1,[r0,#0x1c]		@ write CS0 segment register		
	dsb
				
	orr lr,lr,#0xA0000000		@ udjust return address to flash
	orr r3,r3,#0xA0000000		@ calculate next instruction flash address	
	dsb	
	isb
	mov pc,r3			@ jump to flash

run_from_flash:
	    
flash_remap_end:
	mov r5,lr

#ifndef  CONFIG_RTSM_ONLY
	bl  icache_enable
	bl  clk_init			@ setup pll		
	bl  ddr3_init			@ setup ddr0		
#endif

	mov lr,r5
exit:	
	mov pc, lr
 
//==================================================================
// Enable caches
// Caches are controlled by the System Control Register:
// I-cache is controlled by bit 12
//==================================================================
icache_enable:
    mrc     p15, 0, r0, c1, c0, 0 @ read CP15 register
    orr     r0, r0, #(0x1 <<12)   @ enable I Cache
    mcr     p15, 0, r0, c1, c0, 0 @ write CP15 register 1
    dsb
    isb
    mov     pc, lr

// Added from sys_cortex_a9.s from T4K diagnostic code
.globl sys_memset32
sys_memset32:
    STMFD   sp!, {r4, r5, r6, r7, r8, r9, r10, r11}
    MOV     r4, r2
    MOV     r5, r2
    MOV     r6, r3
    MOV     r7, r3
    MOV     r8, r2
    MOV     r9, r2
    MOV     r10, r3
    MOV     r11, r3
sys_memset32_ck_done:
    CMP     r0, r1
    BGE     sys_memset32_exit
    STMIA   r0!, {r4, r5, r6, r7, r8, r9, r10, r11}
    B       sys_memset32_ck_done
sys_memset32_exit:
    LDMFD   sp!, {r4, r5, r6, r7, r8, r9, r10, r11}
    MOV     pc,lr

.globl sys_compare32    
sys_compare32:
    STMFD   sp!, {r4, r5, r6, r7, r8, r9, r10, r11}
compare_loop:
    LDMIA   r0, {r4, r5, r6, r7, r8, r9, r10, r11}
    CMP     r4, r2
    BNE     compare_error
    ADD     r0, r0, #4
    CMP     r5, r2
    BNE     compare_error
    ADD     r0, r0, #4
    CMP     r6, r3
    BNE     compare_error
    ADD     r0, r0, #4
    CMP     r7, r3
    BNE     compare_error
    ADD     r0, r0, #4
    CMP     r8, r2
    BNE     compare_error
    ADD     r0, r0, #4
    CMP     r9, r2
    BNE     compare_error
    ADD     r0, r0, #4
    CMP     r10, r3
    BNE     compare_error
    ADD     r0, r0, #4
    CMP     r11, r3
    BNE     compare_error
    ADD     r0, r0, #4
    CMP     r0, r1
    BLO     compare_loop
    
    LDMFD   sp!, {r4, r5, r6, r7, r8, r9, r10, r11}
    MOV     pc,lr

compare_error:
    LDMFD   sp!, {r4, r5, r6, r7, r8, r9, r10, r11}
    MOV     pc,lr
