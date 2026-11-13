.section .init_array,"aw",%init_array
	.p2align 4, 0
	.globl __init_array_start
__init_array_start:
	.globl __init_array_end
__init_array_end:
	.section .ctors,"aw",%progbits
	.p2align 4, 0
	.globl __CTOR_LIST__
__CTOR_LIST__:
	.globl __CTOR_END__
__CTOR_END__:
	.section .dtors,"aw",%progbits
	.p2align 4, 0
	.globl __DTOR_LIST__
__DTOR_LIST__:
	.globl __DTOR_END__
__DTOR_END__:
	.section .fini_array,"aw",%fini_array
	.p2align 4, 0
	.globl __fini_array_start
__fini_array_start:
	.globl __fini_array_end
__fini_array_end:

#if defined(__ELF__) && defined(__linux__)
	.section .note.GNU-stack,"",%progbits
#endif
