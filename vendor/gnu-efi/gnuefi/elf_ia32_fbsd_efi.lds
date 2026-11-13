OUTPUT_FORMAT("elf32-i386-freebsd", "elf32-i386-freebsd", "elf32-i386-freebsd")
OUTPUT_ARCH(i386)
ENTRY(_start)
SECTIONS
{
  . = 0;
  ImageBase = .;

  .hash : { *(.hash) }
  .gnu.hash : { *(.gnu.hash) }
  . = ALIGN(4096);
  .text :
  {
   _text = .;
   *(.text)
   *(.text.*)
   *(.gnu.linkonce.t.*)
   . = ALIGN(16);
  }
  _etext = .;
  _text_size = _etext - _text;
  . = ALIGN(4096);
  .sdata :
  {
   _data = .;
   *(.got.plt)
   *(.got)
   *(.srodata)
   *(.sdata)
   *(.sbss)
   *(.scommon)
  }
  . = ALIGN(4096);
  .data :
  {
   *(.rodata*)
   *(.data)
   *(.data1)
   *(.data.*)
   *(.sdata)
   *(.got.plt)
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

   *(.sbss)
   *(.scommon)
   *(.dynbss)
   *(.bss)
   *(COMMON)
  }
  .note.gnu.build-id : { *(.note.gnu.build-id) }

  . = ALIGN(4096);
  .dynamic  : { *(.dynamic) }
  . = ALIGN(4096);
  .rel :
  {
    *(.rel.data)
    *(.rel.data.*)
    *(.rel.got)
    *(.rel.stab)
    *(.rel.init_array*)
    *(.rel.fini_array*)
    *(.rel.ctors*)
    *(.rel.dtors*)
    *(.data.rel.ro.local)
    *(.data.rel.local)
    *(.data.rel.ro)
    *(.data.rel*)
  }
  _edata = .;
  _data_size = _edata - _etext;
  . = ALIGN(4096);
  .reloc :
  {
    *(.reloc)
  }
  . = ALIGN(4096);
  .dynsym   : { *(.dynsym) }
  . = ALIGN(4096);
  .dynstr   : { *(.dynstr) }
  . = ALIGN(4096);
  /DISCARD/ :
  {
    *(.rel.reloc)
    *(.eh_frame)
    *(.note.GNU-stack)
  }
  .comment 0 : { *(.comment) }
}
