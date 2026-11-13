#ifndef EFI_SUBSYSTEM
#define EFI_SUBSYSTEM 10
#endif

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
	.2byte	0x5064
	.2byte	4
	.4byte	0
	.4byte	0
	.4byte	0
	.2byte	section_table - optional_header
	.2byte	0x206

optional_header:
	.2byte	0x20b
	.byte	0x02
	.byte	0x14
	.4byte	_text_size - ImageBase
	.4byte	_alldata_size - ImageBase
	.4byte	0
	.4byte	_start - ImageBase
	.4byte	_text - ImageBase

extra_header_fields:
	.8byte	0
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

	.4byte	_text - ImageBase
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
	.4byte	_text_vsize - ImageBase
	.4byte	_text - ImageBase
	.4byte	_text_size - ImageBase
	.4byte	_text - ImageBase
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

	.text
	.globl _start
	.type _start,%function
_start:
	addi		sp, sp, -24
	sd		a0, 0(sp)
	sd		a1, 8(sp)
	sd		ra, 16(sp)
	lla		a0, ImageBase
	lla		a1, _DYNAMIC
	call		_relocate
	bne		a0, zero, 0f
	ld		a1, 8(sp)
	ld		a0, 0(sp)
	call		_entry
	ld		ra, 16(sp)
0:	addi		sp, sp, 24
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
