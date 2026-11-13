#include "edk2asm.h"

  .text
  .align 2
  GCC_ASM_EXPORT(__aeabi_ldivmod)

ASM_PFX(__aeabi_ldivmod):
    push     {r4,lr}
    asrs     r4,r1,#1
    eor      r4,r4,r3,LSR #1
    bpl      L_Test1
    rsbs     r0,r0,#0
    rsc      r1,r1,#0
L_Test1:
    tst      r3,r3
    bpl      L_Test2
    rsbs     r2,r2,#0
    rsc      r3,r3,#0
L_Test2:
    bl       ASM_PFX(__aeabi_uldivmod)
    tst      r4,#0x40000000
    beq      L_Test3
    rsbs     r0,r0,#0
    rsc      r1,r1,#0
L_Test3:
    tst      r4,#0x80000000
    beq      L_Exit
    rsbs     r2,r2,#0
    rsc      r3,r3,#0
L_Exit:
    pop      {r4,pc}

#if defined(__ELF__) && defined(__linux__)
	.section .note.GNU-stack,"",%progbits
#endif
