OUTPUT_FORMAT("elf64-tradlittlemips", "elf64-tradbigmips", "elf64-tradlittlemips")
OUTPUT_ARCH(mips)
ENTRY(_start)
SECTIONS
{
  .text 0x0 : {
    _text = .;
    *(.text.head)
    *(.text)
    *(.text.*)
    *(.gnu.linkonce.t.*)
    *(.plt)
    . = ALIGN(16);
  }
  _etext = .;
  _text_size = _etext - _text;
  . = DATA_SEGMENT_ALIGN (CONSTANT (MAXPAGESIZE), CONSTANT (COMMONPAGESIZE));
  .dynamic  : { *(.dynamic) }
  . = ALIGN(4096);
  .data :
  {
   _data = .;
   *(.sdata)
   *(.data)
   *(.data1)
   *(.data.*)
   *(.got.plt)
   HIDDEN (_gp = ALIGN (16) + 0x7ff0);
   *(.got)

   . = ALIGN(16);
   __init_array_start = .;
   *(SORT(.init_array.*))
   *(.init_array)
   __init_array_end = .;
  . = ALIGN(16);
   __CTOR_LIST__ = .;
   *(SORT(.ctors.*))
   *(.ctors)
   __CTOR_END__ = .;
  . = ALIGN(16);
   __DTOR_LIST__ = .;
   *(SORT(.dtors.*))
   *(.dtors)
   __DTOR_END__ = .;
   . = ALIGN(16);
   __fini_array_start = .;
   *(SORT(.fini_array.*))
   *(.fini_array)
   __fini_array_end = .;

   . = ALIGN(16);
   _bss = .;
   *(.sbss)
   *(.scommon)
   *(.dynbss)
   *(.bss*)
   *(COMMON)
   . = ALIGN(16);
   _bss_end = .;
  }

  . = ALIGN(4096);
  .rel :
  {
    *(.rel.text*)
    *(.rel.data*)
    *(.rel.got)
    *(.rel.dyn)
    *(.rel.stab)
    *(.rel.init_array*)
    *(.rel.fini_array*)
    *(.rel.ctors*)
    *(.rel.dtors*)
  }
  . = ALIGN(4096);
  .rel.plt : { *(.rel.plt) }
  . = ALIGN(4096);
  .rodata : { *(.rodata*) }
  _edata = .;
  _data_size = _edata - _etext;

  . = ALIGN(4096);
  .dynsym   : { *(.dynsym) }
  . = ALIGN(4096);
  .dynstr   : { *(.dynstr) }
  . = ALIGN(4096);
  .note.gnu.build-id : { *(.note.gnu.build-id) }
  . = DATA_SEGMENT_END (.);
  /DISCARD/ :
  {
    *(.rel.reloc)
    *(.eh_frame)
    *(.MIPS.abiflags)
    *(.note.GNU-stack)
  }
  .comment 0 : { *(.comment) }
}
