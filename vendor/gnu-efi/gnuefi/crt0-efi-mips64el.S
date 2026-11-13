.section	.text.head

	.globl	ImageBase
ImageBase:
	.ascii	"MZ"
	.skip	58
	.4byte	pe_header - ImageBase
pe_header:
	.ascii	"PE"
	.2byte 	0
coff_header:
	.2byte	0x166
	.2byte	2
	.4byte	0
	.4byte	0
	.4byte	0
	.2byte	section_table - optional_header
	.2byte	0x206

optional_header:
	.2byte	0x20b
	.byte	0x02
	.byte	0x14
	.4byte	_edata - _start
	.4byte	0
	.4byte	0
	.4byte	_start - ImageBase
	.4byte	_start - ImageBase

extra_header_fields:
	.8byte	0
	.4byte	0x20
	.4byte	0x8
	.2byte	0
	.2byte	0
	.2byte	0
	.2byte	0
	.2byte	0
	.2byte	0
	.4byte	0

	.4byte	_edata - ImageBase

	.4byte	_start - ImageBase
	.4byte	0
	.2byte	EFI_SUBSYSTEM
	.2byte	0
	.8byte	0
	.8byte	0
	.8byte	0
	.8byte	0
	.4byte	0
	.4byte	0x10

	.8byte	0
	.8byte	0
	.8byte	0
	.8byte	0
	.8byte	0
	.8byte	0
	.8byte	0
	.8byte	0
	.8byte	0
	.8byte	0
	.8byte	0
	.8byte	0
	.8byte	0
	.8byte	0
	.8byte	0
	.8byte	0

section_table:

	.ascii	".reloc"
	.byte	0
	.byte	0
	.4byte	0
	.4byte	0
	.4byte	0
	.4byte	0
	.4byte	0
	.4byte	0
	.2byte	0
	.2byte	0
	.4byte	0x42100040

	.ascii	".text"
	.byte	0
	.byte	0
	.byte	0
	.4byte	_edata - _start
	.4byte	_start - ImageBase
	.4byte	_edata - _start
	.4byte	_start - ImageBase

	.4byte	0
	.4byte	0
	.2byte	0
	.2byte	0
	.4byte	0xe0500020

	.set		push
	.set		noreorder
	.align		4

	.globl	_start
	.ent	_start
	.type	_start, @function
_start:
	daddiu		$sp, -32
	sd		$ra, ($sp)

	.align		3
	bal		1f
	sd		$gp, 8($sp)
_pc:
	.dword		_gp
	.dword		_DYNAMIC
	.dword		_relocate
1:

	ld		$gp, ($ra)
	dli		$t0, _pc
	dsubu		$gp, $t0
	daddu		$gp, $ra

	sd		$a0, 16($sp)
	sd		$a1, 24($sp)

	move		$a2, $a0

	move		$a3, $a1

	dli		$t1, ImageBase - _pc
	daddu		$a0, $ra, $t1

	ld		$t1, 8($ra)
	dsubu		$t1, $t0
	daddu		$a1, $ra, $t1

	ld		$t1, 16($ra)
	dsubu		$t1, $t0
	daddu		$t9, $ra, $t1
	jalr		$t9
	nop
	bnez		$v0, 1b
	nop

	ld		$a0, 16($sp)

	dla		$t9, _entry
	jalr		$t9

	ld		$a1, 24($sp)

1:
	ld		$gp, 8($sp)
	ld		$ra, ($sp)
	jr		$ra
	daddiu		$sp, 32
	.end		_start

	.set		pop

#if defined(__ELF__) && defined(__linux__)
	.section .note.GNU-stack,"",%progbits
#endif
