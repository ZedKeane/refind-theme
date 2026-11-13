#include "edk2asm.h"

.text
GCC_ASM_EXPORT(__aeabi_lmul)
ASM_PFX(__aeabi_lmul):
  stmdb   sp!, {lr}
  mov     lr, r0
  umull   r0, ip, r2, lr
  mla     r1, r2, r1, ip
  mla     r1, r3, lr, r1
  ldmia   sp!, {pc}

#if defined(__ELF__) && defined(__linux__)
	.section .note.GNU-stack,"",%progbits
#endif
