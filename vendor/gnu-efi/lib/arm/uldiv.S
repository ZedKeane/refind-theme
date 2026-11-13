#include "edk2asm.h"

  .text
  .align 2
  GCC_ASM_EXPORT(__aeabi_uldivmod)

ASM_PFX(__aeabi_uldivmod):
  stmdb   sp!, {r4, r5, r6, lr}
  mov     r4, r1
  mov     r5, r0
  mov     r6, #0
  orrs    ip, r3, r2, lsr #31
  bne     ASM_PFX(__aeabi_uldivmod_label1)
  tst     r2, r2
  beq     ASM_PFX(_ll_div0)
  movs    ip, r2, lsr #15
  addeq   r6, r6, #16
  mov     ip, r2, lsl r6
  movs    lr, ip, lsr #23
  moveq   ip, ip, lsl #8
  addeq   r6, r6, #8
  movs    lr, ip, lsr #27
  moveq   ip, ip, lsl #4
  addeq   r6, r6, #4
  movs    lr, ip, lsr #29
  moveq   ip, ip, lsl #2
  addeq   r6, r6, #2
  movs    lr, ip, lsr #30
  moveq   ip, ip, lsl #1
  addeq   r6, r6, #1
  b       ASM_PFX(_ll_udiv_small)
ASM_PFX(__aeabi_uldivmod_label1):
  tst     r3, #-2147483648
  bne     ASM_PFX(__aeabi_uldivmod_label2)
  movs    ip, r3, lsr #15
  addeq   r6, r6, #16
  mov     ip, r3, lsl r6
  movs    lr, ip, lsr #23
  moveq   ip, ip, lsl #8
  addeq   r6, r6, #8
  movs    lr, ip, lsr #27
  moveq   ip, ip, lsl #4
  addeq   r6, r6, #4
  movs    lr, ip, lsr #29
  moveq   ip, ip, lsl #2
  addeq   r6, r6, #2
  movs    lr, ip, lsr #30
  addeq   r6, r6, #1
  rsb     r3, r6, #32
  moveq   ip, ip, lsl #1
  orr     ip, ip, r2, lsr r3
  mov     lr, r2, lsl r6
  b       ASM_PFX(_ll_udiv_big)
ASM_PFX(__aeabi_uldivmod_label2):
  mov     ip, r3
  mov     lr, r2
  b       ASM_PFX(_ll_udiv_ginormous)

ASM_PFX(_ll_udiv_small):
  cmp     r4, ip, lsl #1
  mov     r3, #0
  subcs   r4, r4, ip, lsl #1
  addcs   r3, r3, #2
  cmp     r4, ip
  subcs   r4, r4, ip
  adcs    r3, r3, #0
  add     r2, r6, #32
  cmp     r2, #32
  rsb     ip, ip, #0
  bcc     ASM_PFX(_ll_udiv_small_label1)
  orrs    r0, r4, r5, lsr #30
  moveq   r4, r5
  moveq   r5, #0
  subeq   r2, r2, #32
ASM_PFX(_ll_udiv_small_label1):
  mov     r1, #0
  cmp     r2, #16
  bcc     ASM_PFX(_ll_udiv_small_label2)
  movs    r0, r4, lsr #14
  moveq   r4, r4, lsl #16
  addeq   r1, r1, #16
ASM_PFX(_ll_udiv_small_label2):
  sub     lr, r2, r1
  cmp     lr, #8
  bcc     ASM_PFX(_ll_udiv_small_label3)
  movs    r0, r4, lsr #22
  moveq   r4, r4, lsl #8
  addeq   r1, r1, #8
ASM_PFX(_ll_udiv_small_label3):
  rsb     r0, r1, #32
  sub     r2, r2, r1
  orr     r4, r4, r5, lsr r0
  mov     r5, r5, lsl r1
  cmp     r2, #1
  bcc     ASM_PFX(_ll_udiv_small_label5)
  sub     r2, r2, #1
  and     r0, r2, #7
  eor     r0, r0, #7
  adds    r0, r0, r0, lsl #1
  add     pc, pc, r0, lsl #2
  nop
ASM_PFX(_ll_udiv_small_label4):
  adcs    r5, r5, r5
  adcs    r4, ip, r4, lsl #1
  rsbcc   r4, ip, r4
  adcs    r5, r5, r5
  adcs    r4, ip, r4, lsl #1
  rsbcc   r4, ip, r4
  adcs    r5, r5, r5
  adcs    r4, ip, r4, lsl #1
  rsbcc   r4, ip, r4
  adcs    r5, r5, r5
  adcs    r4, ip, r4, lsl #1
  rsbcc   r4, ip, r4
  adcs    r5, r5, r5
  adcs    r4, ip, r4, lsl #1
  rsbcc   r4, ip, r4
  adcs    r5, r5, r5
  adcs    r4, ip, r4, lsl #1
  rsbcc   r4, ip, r4
  adcs    r5, r5, r5
  adcs    r4, ip, r4, lsl #1
  rsbcc   r4, ip, r4
  adcs    r5, r5, r5
  adcs    r4, ip, r4, lsl #1
  sub     r2, r2, #8
  tst     r2, r2
  rsbcc   r4, ip, r4
  bpl     ASM_PFX(_ll_udiv_small_label4)
ASM_PFX(_ll_udiv_small_label5):
  mov     r2, r4, lsr r6
  bic     r4, r4, r2, lsl r6
  adcs    r0, r5, r5
  adc     r1, r4, r4
  add     r1, r1, r3, lsl r6
  mov     r3, #0
  ldmia   sp!, {r4, r5, r6, pc}

ASM_PFX(_ll_udiv_big):
  subs    r0, r5, lr
  mov     r3, #0
  sbcs    r1, r4, ip
  movcs   r5, r0
  movcs   r4, r1
  adcs    r3, r3, #0
  subs    r0, r5, lr
  sbcs    r1, r4, ip
  movcs   r5, r0
  movcs   r4, r1
  adcs    r3, r3, #0
  subs    r0, r5, lr
  sbcs    r1, r4, ip
  movcs   r5, r0
  movcs   r4, r1
  adcs    r3, r3, #0
  mov     r1, #0
  rsbs    lr, lr, #0
  rsc     ip, ip, #0
  cmp     r6, #16
  bcc     ASM_PFX(_ll_udiv_big_label1)
  movs    r0, r4, lsr #14
  moveq   r4, r4, lsl #16
  addeq   r1, r1, #16
ASM_PFX(_ll_udiv_big_label1):
  sub     r2, r6, r1
  cmp     r2, #8
  bcc     ASM_PFX(_ll_udiv_big_label2)
  movs    r0, r4, lsr #22
  moveq   r4, r4, lsl #8
  addeq   r1, r1, #8
ASM_PFX(_ll_udiv_big_label2):
  rsb     r0, r1, #32
  sub     r2, r6, r1
  orr     r4, r4, r5, lsr r0
  mov     r5, r5, lsl r1
  cmp     r2, #1
  bcc     ASM_PFX(_ll_udiv_big_label4)
  sub     r2, r2, #1
  and     r0, r2, #3
  rsb     r0, r0, #3
  adds    r0, r0, r0, lsl #1
  add     pc, pc, r0, lsl #3
  nop
ASM_PFX(_ll_udiv_big_label3):
  adcs    r5, r5, r5
  adcs    r4, r4, r4
  adcs    r0, lr, r5
  adcs    r1, ip, r4
  movcs   r5, r0
  movcs   r4, r1
  adcs    r5, r5, r5
  adcs    r4, r4, r4
  adcs    r0, lr, r5
  adcs    r1, ip, r4
  movcs   r5, r0
  movcs   r4, r1
  adcs    r5, r5, r5
  adcs    r4, r4, r4
  adcs    r0, lr, r5
  adcs    r1, ip, r4
  movcs   r5, r0
  movcs   r4, r1
  sub     r2, r2, #4
  adcs    r5, r5, r5
  adcs    r4, r4, r4
  adcs    r0, lr, r5
  adcs    r1, ip, r4
  tst     r2, r2
  movcs   r5, r0
  movcs   r4, r1
  bpl     ASM_PFX(_ll_udiv_big_label3)
ASM_PFX(_ll_udiv_big_label4):
  mov     r1, #0
  mov     r2, r5, lsr r6
  bic     r5, r5, r2, lsl r6
  adcs    r0, r5, r5
  adc     r1, r1, #0
  movs    lr, r3, lsl r6
  mov     r3, r4, lsr r6
  bic     r4, r4, r3, lsl r6
  adc     r1, r1, #0
  adds    r0, r0, lr
  orr     r2, r2, r4, ror r6
  adc     r1, r1, #0
  ldmia   sp!, {r4, r5, r6, pc}

ASM_PFX(_ll_udiv_ginormous):
  subs    r2, r5, lr
  mov     r1, #0
  sbcs    r3, r4, ip
  adc     r0, r1, r1
  movcc   r2, r5
  movcc   r3, r4
  ldmia   sp!, {r4, r5, r6, pc}

ASM_PFX(_ll_div0):
  ldmia   sp!, {r4, r5, r6, lr}
  mov     r0, #0
  mov     r1, #0
  b       ASM_PFX(__aeabi_ldiv0)

ASM_PFX(__aeabi_ldiv0):
  bx      r14

#if defined(__ELF__) && defined(__linux__)
	.section .note.GNU-stack,"",%progbits
#endif
