.text
	.align 4

	.globl _start
	.type _start,%function
_start:
	pushl %ebp
	movl %esp,%ebp

	pushl 12(%ebp)
	pushl  8(%ebp)

	call 0f
0:	popl %eax
	movl %eax,%ebx

	addl $ImageBase-0b,%eax
	addl $_DYNAMIC-0b,%ebx

	pushl %ebx
	pushl %eax
	call _relocate
	popl %ebx
	popl %ebx
	testl %eax,%eax
	jne .exit

	call _entry

.exit:	leave
	ret

	.data
dummy:	.4byte	0

#define IMAGE_REL_ABSOLUTE	0
	.section .reloc
	.4byte	dummy
	.4byte	12
	.2byte	(IMAGE_REL_ABSOLUTE<<12) +  0
	.2byte	(IMAGE_REL_ABSOLUTE<<12) +  0

#if defined(__ELF__) && defined(__linux__)
	.section .note.GNU-stack,"",%progbits
#endif
