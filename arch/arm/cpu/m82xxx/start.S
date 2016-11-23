#include <config.h>
#include <version.h>
#include <asm/arch/hardware.h>


.global _vectors_min_start
_vectors_min_start:
    b reset
    ldr pc, _min_undefined_instruction
    ldr pc, _min_software_interrupt
    ldr pc, _min_prefetch_abort
    ldr pc, _min_data_abort
    ldr pc, _min_not_used
    ldr pc, _min_irq
    ldr pc, _min_fiq

_min_undefined_instruction:
    .word min_undefined_instruction
_min_software_interrupt:
    .word min_software_interrupt
_min_prefetch_abort:
    .word min_prefetch_abort
_min_data_abort:
    .word min_data_abort
_min_not_used:
    .word min_not_used
_min_irq:
    .word min_irq
_min_fiq:
    .word min_fiq
.global _vectors_min_end
_vectors_min_end:


_INIT_STACK:
    .word	IRAM_BASE + 0x800

.equ UART_LSR_THRE, 0x20
.equ VECTOR_BASE_DST, 0x0

.macro print_char char_arg
    ldr r0, =0xFE800000
uart_wait\@:
    ldr r2, [r0, #20] /* get LSR */
    /*ldrb r2, [r0, #23]*/ /* Problem */
    /*ldrb r2, [r0, #20]*/ /* OK*/
    tst r2, #UART_LSR_THRE
    beq uart_wait\@
    ldr r1, =\char_arg
    str r1, [r0]
.endm

.global reset
reset:
    print_char 'R'

    /* Clear the BSS */
    /* C functions can get called (verbose exception handlers) before the BSS*/
    /* is cleared, so do it here as well.*/
    ldr r0, =__bss_start
    ldr r1, =__bss_end
    mov r2, #0
clbss_loop:
    str r2, [r0], #4
    cmp r0, r1
    bne clbss_loop

    /* Relocate CPU exception vectors */
    /* ldr r0, =_start  TODO These vectors aren't working from some reason */
    /* ldr r1, =_vectors_end */
    ldr r0, =_vectors_min_start
    ldr r1, =_vectors_min_end
    mov r2, #VECTOR_BASE_DST
v_copy_loop:
    ldr r3, [r0], #4
    str r3, [r2], #4
    cmp r0, r1
    bne v_copy_loop

    /* print_char 'M' */

    /*
     * segfault to test the exception vectors
     * Exception vectors aren't working
     */
    /* ldr r0, =0xffffffff */
    /* str r2, [r0] */

    /*
     * set the cpu to SVC32 mode
     */
    mrs r0,cpsr
    bic r0,r0,#0x1f
    orr r0,r0,#0xd3
    msr cpsr,r0

/*Commented out block, not required with microloader already setting up things
This block hangs the board once trustzone is enabled as these register are no more accessible*/
    #ldr r0,=0xF4C08000  /*TODO: IRAM remap remove*/
    #ldr r1,[r0]
    #bic r1,r1,#0x2000000 /*TODO: 25th bit IRAM remap*/
    #str r1,[r0]

    /*
     * Enable access to debugger
     */
    /* set A9_AUTHENT_REG */
    #mov    r0,#0xFE000000
    #mov    r1,#0x00070000
    #orr    r0, r0, r1
    #mov    r1,#0xF
    #str     r1, [r0,#0x70]
    /* set DBGEN in Cross Trigger Source Enable Register */
    #mov    r0,#0xF4000000
    #mov    r1,#0x00C00000
    #orr    r0, r0, r1
    #mov     r1, #0x300;
    #str     r1, [r0,#0x100]

    /* set DDR Low and High limit */
    #ldr     r1,=0xB0000000
    #ldr     r0,=0xF4C00010
    #str     r1,[r0]
    #dsb
/*Commented out block ends*/
    /*
     * we do sys-critical inits only at reboot,
     * not when booting from ram!
     */
    bl invalidate_cache

    /* print_char 'C' */

/* The following snippet from old startup code looks important, what does it do ??
 * is this the entry point for the second CPU ??
 *       mov     r0,#0                                      r0 = 0
 *       ldr     r1,=0xE59FF018                             r1 = 0xE59FF018             //this is a magical number, perhaps it's telling the code something?
 *       ldr     r2,=secondary_entry                        r2 = secondary_entry
 *       str     r1,[r0,#0]                                 *(r0 + 0) = r1
 *       str     r2,[r0,#0x20]                              *(r0 + 0x20) = r2
 *       ldr     r2,=0xF4000000                             r2 = 0xF4000000
 *       str     r0,[r2]                                    *(r2) = r0
 *       ldr     r1,=0xF4CF0078		@ CPU reset register    r1 = 0xF4CF0078
 *       str     r0,[r1]                                    *(r1) = r0
 */

/*
 *************************************************************************
 *
 * CPU_init_critical registers
 *
 * setup important registers
 * setup memory timing
 *
 *************************************************************************
 */

cpu_init_crit:
    /*
     * disable MMU stuff and caches
     */
    mrc	p15, 0, r0, c1, c0, 0
    bic	r0, r0, #0x00002000	@ clear bits 13 (--V-)
    bic	r0, r0, #0x00000007	@ clear bits 2:0 (-CAM)
    orr	r0, r0, #0x00000002	@ set bit 1 (--A-) Align
    mcr	p15, 0, r0, c1, c0, 0
    dsb	
    isb

    /*
     * Jump to board specific initialization...
     * Go here to bump up clock rate and handle
     * wake up conditions.
     */
    ldr sp, _INIT_STACK     @ initial stack from internal memory
    bl lowlevel_init         @ go setup pll,mux,memory   !!!! TODO !!!! how are we going to come back to call _main ??

    /* print_char 'a' */
    bl _main  /* In crt0.S, this goes on to relocate */
    /* TODO do we need to catch a return to here */
    /* print_char 'F' */

/*
 *************************************************************************
 *
 * invalidate_cache
 *
 * TODO requires a comment.
 *
 *************************************************************************
 */

invalidate_cache:
    dmb
    mcr     p15, 0, r0, c8, c7, 0           @ Invalidate entire Unified TLB
    mcr     p15, 0, r0, c7, c5, 0           @ Invalidate ICache
    mrc     p15, 1, r0, c0, c0, 1           @ read clidr
    ands    r3, r0, #0x7000000              @ extract loc from clidr
    mov     r3, r3, lsr #23                 @ left align loc bit field
    beq     finished                        @ if loc is 0, then no need to clean
    mov     r10, #0                         @ start clean at cache level 0
loop1:
    add     r2, r10, r10, lsr #1            @ work out 3x current cache level
    mov     r1, r0, lsr r2                  @ extract cache type bits from clidr
    and     r1, r1, #7                      @ mask of the bits for current cache only
    cmp     r1, #2                          @ see what cache we have at this level
    blt     skip                            @ skip if no cache, or just i-cache
    mcr     p15, 2, r10, c0, c0, 0          @ select current cache level in cssr
    isb                                     @ isb to sych the new cssr&csidr
    mrc     p15, 1, r1, c0, c0, 0           @ read the new csidr
    and     r2, r1, #7                      @ extract the length of the cache lines
    add     r2, r2, #4                      @ add 4 (line length offset)
    ldr     r4, =0x3ff
    ands    r4, r4, r1, lsr #3              @ find maximum number on the way size
    clz     r5, r4                          @ find bit position of way size increment
    ldr     r7, =0x7fff
    ands    r7, r7, r1, lsr #13             @ extract max number of the index size
loop2:
    mov     r9, r4                          @ create working copy of max way size
loop3:
    orr     r11, r10, r9, lsl r5            @ factor way and cache number into r11
    orr     r11, r11, r7, lsl r2            @ factor index number into r11
    mcr     p15, 0, r11, c7, c6, 2          @ invalidate by set/way
    subs    r9, r9, #1                      @ decrement the way
    bge     loop3
    subs    r7, r7, #1                      @ decrement the index
    bge     loop2
skip:
    add     r10, r10, #2                    @ increment cache number
    cmp     r3, r10
    bgt     loop1
finished:
    mov     r10, #0                         @ swith back to cache level 0
    mcr     p15, 2, r10, c0, c0, 0          @ select current cache level in cssr
    dsb
    isb
    mov     pc, lr

/*
 *************************************************************************
 *
 * fixup_cpu (currently unused TODO look into why)
 *
 * TODO requires a comment. 
 *
 *************************************************************************
 */

fixup_cpu:
    mrc     p15, 0, r10, c15, c0, 1         @ 
    orr     r10, r10, #(1 << 12)            @ disable outstanding access
    orr     r10, r10, #(1 << 20)            @ disable PLD
    orr     r10, r10, #(1 << 24)            @ disable PLI
    mcr     p15, 0, r10, c15, c0, 1         @
    dsb
    mrc     p15, 0, r10, c1, c0, 1          @ errata #571620
    bic     r10, r10, #(1 << 2)             @ disable L1 automatic data prefecher
    bic     r10, r10, #(1 << 1)             @ disable L2 automatic data prefecher
    mcr     p15, 0, r10, c1, c0, 1          @
    dsb
    mov     pc, lr

.global c_runtime_cpu_setup
c_runtime_cpu_setup:
    mov     pc, lr

min_undefined_instruction:
    print_char 'U'
    mov r12, lr
    bl print_r12
    b .
min_software_interrupt:
    print_char 'S'
    b .
min_prefetch_abort:
    print_char 'P'
    b .
min_data_abort:
    print_char 'D'
    mov r12, lr
    bl print_r12
    b .
min_not_used:
    print_char 'N'
    b .
min_irq:
    print_char 'I'
    b .
min_fiq:
    print_char 'F'
    b .

print_r12:
    /* Print ascii for value in register r12                */
    /* Modified registers: r0,r1,r2,r3                      */
    ldr r0, =0xFE800000
    ldr r1, =0xf0000000         /* mask                     */
    ldr r2, =28                 /* shift                    */
char_loop:
    and r3, r12, r1             /* mask the value           */
    lsr r3, r3, r2              /* r3 = r3 >> r2            */
    cmp r3, #0xa
    addlt r3, r3, #48           /* if(r3 < 10) r3 = r3 + '0'*/
    addge r3, r3, #55           /* if(r3 >= 10)r3 = r3 + 'A'*/
    str r3, [r0]                /* output char              */
    lsr r1, r1, #4              /* r1 = r1 >> 4             */
    sub r2, r2, #4              /* r2 = r2 - 4              */
uart_wait_r12:
    ldr r3, [r0, #20]           /* reuse r3 for delay       */
    tst r3, #UART_LSR_THRE      /* wait for UART LSR_THRE   */
    beq uart_wait_r12
    cmp r1, #0
    bne char_loop
    mov pc, lr

/*.global abort*/
/*abort:*/
    /* Probably not the best way to do this. */
/*    ldr r0, =0xffffffff */
/*    str r2, [r0] */

