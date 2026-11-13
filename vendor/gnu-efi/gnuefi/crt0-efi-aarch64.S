.text
	.align		12

	.globl _start
	.type _start,%function
_start:
	stp		x29, x30, [sp, #-32]!
	mov		x29, sp

	stp		x0, x1, [sp, #16]
	mov		x2, x0
	mov		x3, x1
	adr		x0, ImageBase
	adrp		x1, _DYNAMIC
	add		x1, x1, #:lo12:_DYNAMIC
	bl		_relocate
	cbnz		x0, 0f

	ldp		x0, x1, [sp, #16]
	bl		_entry

0:	ldp		x29, x30, [sp], #32
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
