.text
	.align 4

	.globl _start
	.type _start,%function
_start:
	subq $8, %rsp
	pushq %rcx
	pushq %rdx

0:
	lea ImageBase(%rip), %rdi
	lea _DYNAMIC(%rip), %rsi

	popq %rcx
	popq %rdx
	pushq %rcx
	pushq %rdx
	call _relocate

	popq %rdi
	popq %rsi

	call _entry
	addq $8, %rsp

.exit:
	ret

	.data
dummy:	.4byte	0

#define IMAGE_REL_ABSOLUTE	0
	.section .reloc, "a"
label1:
	.4byte	dummy-label1
	.4byte	12
	.2byte	(IMAGE_REL_ABSOLUTE<<12) +  0
	.2byte	(IMAGE_REL_ABSOLUTE<<12) +  0

#if defined(__ELF__) && defined(__linux__)
	.section .note.GNU-stack,"",%progbits
#endif
