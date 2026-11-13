.text
	.arm
	.globl	setjmp
	.type	setjmp, %function
setjmp:
	mov	r3, r13
	stmia	r0, {r3-r12,r14}
	eor	r0, r0, r0
	bx	lr

	.globl	longjmp
	.type	longjmp, %function
longjmp:
	ldmia	r0, {r3-r12,r14}

#if defined(__ELF__) && defined(__linux__)
	.section .note.GNU-stack,"",%progbits
#endif
