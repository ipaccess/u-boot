  .globl fastcall_tz
  .arch armv7-a
  .arch_extension sec
  .arm
  .code 32 

   @PRESERVE8
   @AREA BOOT_FAST_CALL, CODE, READONLY


fastcall_tz:
  stmfd sp!, {r1-r12, lr}
  orr     r0, r0, #(2 << 8)   /*; val 2 in bits 8-11 means fastcall*/
  push    {r4-r5}
  add     lr, sp, #60
  ldmia   lr, {r4-r5}
  smc     0x0
  pop     {r4-r5}
  ldmfd sp!, {r1-r12, lr}   /*; r0 has return value.*/
  bx lr  
