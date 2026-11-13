.text
	.align		12
	.globl	_start
	.type	_start, @function
_start:
	addi.d	  $sp, $sp, -24
	st.d	  $ra, $sp, 0
	st.d	  $a0, $sp, 8
	st.d 	  $a1, $sp, 16

	move	  $a2, $a0
	move	  $a3, $a1
	la.local  $a0, ImageBase
	la.local  $a1, _DYNAMIC
	bl        _relocate
	bnez	  $a0, 0f

	ld.d	  $a0, $sp, 8
	ld.d	  $a1, $sp, 16
	bl        _entry

0:	ld.d	  $ra, $sp, 0
	addi.d	  $sp, $sp, 24
	jr	      $ra

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
