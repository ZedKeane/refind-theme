#include "edk2asm.h"

.text
.align 2
GCC_ASM_EXPORT(__aeabi_llsl)

ASM_PFX(__aeabi_llsl):
    subs     r3,r2,#0x20
    bpl      1f
    rsb      r3,r2,#0x20
    lsl      r1,r1,r2
    orr      r1,r1,r0,lsr r3
    lsl      r0,r0,r2
    bx       lr
1:
    lsl      r1,r0,r3
    mov      r0,#0
    bx       lr

#if defined(__ELF__) && defined(__linux__)
	.section .note.GNU-stack,"",%progbits
#endif
