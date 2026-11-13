#include "edk2asm.h"

.text
.align 2
GCC_ASM_EXPORT(__aeabi_llsr)

ASM_PFX(__aeabi_llsr):
    subs     r3,r2,#0x20
    bpl      1f
    rsb      r3,r2,#0x20
    lsr      r0,r0,r2
    orr      r0,r0,r1,lsl r3
    lsr      r1,r1,r2
    bx       lr
1:
    lsr      r0,r1,r3
    mov      r1,#0
    bx       lr

#if defined(__ELF__) && defined(__linux__)
	.section .note.GNU-stack,"",%progbits
#endif
