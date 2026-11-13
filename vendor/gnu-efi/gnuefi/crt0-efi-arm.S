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
	.2byte	0x1c2
	.2byte	4
	.4byte	0
	.4byte	0
	.4byte	0
	.2byte	section_table - optional_header
	.2byte	0x306

optional_header:
	.2byte	0x10b
	.byte	0x02
	.byte	0x14
	.4byte	_etext - _start
	.4byte	_alldata_size - ImageBase
	.4byte	0
	.4byte	_start - ImageBase
	.4byte	_start - ImageBase
	.4byte	_reloc - ImageBase

extra_header_fields:
	.4byte	0
	.4byte	0x1000
	.4byte	0x1000
	.2byte	0
	.2byte	0
	.2byte	0
	.2byte	0
	.2byte	0
	.2byte	0
	.4byte	0

	.4byte	_image_end - ImageBase

	.4byte	_start - ImageBase
	.4byte	0
	.2byte	EFI_SUBSYSTEM
	.2byte	0
	.4byte	0
	.4byte	0
	.4byte	0
	.4byte	0
	.4byte	0
	.4byte	0x10

	.8byte	0
	.8byte	0
	.8byte	0
	.8byte	0
	.8byte	0
	.4byte	_reloc - ImageBase
	.4byte	_reloc_vsize - ImageBase
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

	.ascii	".text\0\0\0"
	.4byte	_evtext - _start
	.4byte	_start - ImageBase
	.4byte	_etext - _start
	.4byte	_start - ImageBase
	.4byte	0
	.4byte	0
	.2byte	0
	.2byte	0
	.4byte	0x60000020

	.ascii	".reloc\0\0"
	.4byte	_reloc_vsize - ImageBase
	.4byte	_reloc - ImageBase
	.4byte	_reloc_size - ImageBase
	.4byte	_reloc - ImageBase
	.4byte	0
	.4byte	0
	.2byte	0
	.2byte	0
	.4byte	0x42000040

	.ascii	".data\0\0\0"
	.4byte	_data_vsize - ImageBase
	.4byte	_data - ImageBase
	.4byte	_data_size - ImageBase
	.4byte	_data - ImageBase
	.4byte	0
	.4byte	0
	.2byte	0
	.2byte	0
	.4byte	0xC0000040

	.ascii	".rodata\0"
	.4byte	_rodata_vsize - ImageBase
	.4byte	_rodata - ImageBase
	.4byte	_rodata_size - ImageBase
	.4byte	_rodata - ImageBase
	.4byte	0
	.4byte	0
	.2byte	0
	.2byte	0
	.4byte	0x40000040

.balign 256
.globl	_start
.type _start,%function
_start:
	stmfd		sp!, {r0-r2, lr}

	mov		r2, r0
	mov		r3, r1
	adr		r1, .L_DYNAMIC
	ldr		r0, [r1]
	add		r1, r0, r1
	adr		r0, ImageBase
	bl		_relocate
	teq		r0, #0
	bne		0f

	ldmfd		sp, {r0-r1}
	bl		_entry

0:	add		sp, sp, #12
	ldr		pc, [sp], #4

.L_DYNAMIC:
	.4byte		_DYNAMIC - .

	.data
dummy:	.4byte	0

#define IMAGE_REL_ABSOLUTE	0
	.section .areloc
	.4byte	dummy
	.4byte	12
	.2byte	(IMAGE_REL_ABSOLUTE<<12) +  0
	.2byte	(IMAGE_REL_ABSOLUTE<<12) +  0

#if defined(__ELF__) && defined(__linux__)
	.section .note.GNU-stack,"",%progbits
#endif
