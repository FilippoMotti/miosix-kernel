ENTRY(_start)

SECTIONS
{
  . = 2M;

  .text BLOCK(4K) : ALIGN(4K)
  {
      *(.multiboot)
      *(.text)
      *(.text.*)
      _etext = .;
  }

  .rodata BLOCK(4K) : ALIGN(4K)
  {
      *(.rodata)
      *(.rodata.*)
  }

  .data BLOCK(4K) : ALIGN(4K)
  {
      _data = .;
      *(.data)
      *(.data.*)
      _edata = .;
  }

  .bss BLOCK(4K) : ALIGN(4K)
  {
      _bss_start = .;
      *(COMMON)
      *(.bss)
      *(.bss.*)
      _bss_end = .;
      _end = .;
  }
  _heap_end = _end + 16M;
  .init_array BLOCK(4K) : ALIGN(4K)
  {
      __miosix_init_array_start = .;
      *(.miosix_init_array)
      __miosix_init_array_end = .;

      __preinit_array_start = .;
      *(.preinit_array)
      *(.preinit_array.*)
      __preinit_array_end = .;

      __init_array_start = .;
      *(.init_array)
      *(.init_array.*)
      __init_array_end = .;

      _ctor_start = .;
      *(.ctors)
      *(.ctors.*)
      _ctor_end = .;
  }
}
