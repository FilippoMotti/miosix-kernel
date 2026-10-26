ENTRY(_start)

SECTIONS
{
  . = 2M;

  .text BLOCK(4K) : ALIGN(4K)
  {
      _text = .;
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

  .eh_frame_hdr BLOCK(4K) : ALIGN(4K)
  {
      __GNU_EH_FRAME_HDR = .;
      *(.eh_frame_hdr)
      __GNU_EH_FRAME_HDR_END = .;
  }
  .eh_frame BLOCK(4K) : ALIGN(4K)
  {
      KEEP(*(.eh_frame))
      KEEP(*(.eh_frame.*))
  }

  .gcc_except_table BLOCK(4K) : ALIGN(4K)
  {
      *(.gcc_except_table)
      *(.gcc_except_table.*)
  }
  .data BLOCK(4K) : ALIGN(4K)
  {
      _data = .;
      *(.data)
      *(.data.*)
      _edata = .;
  }
  .init_array BLOCK(4K) : ALIGN(4K)
  {
      __miosix_init_array_start = .;
      KEEP (*(SORT(.miosix_init_array.*)))
      KEEP (*(.miosix_init_array))
      __miosix_init_array_end = .;

      __preinit_array_start = .;
      KEEP (*(.preinit_array))
      __preinit_array_end = .;

      __init_array_start = .;
      KEEP (*(SORT(.init_array.*)))
      KEEP (*(.init_array))
      __init_array_end = .;

      _ctor_start = .;
      KEEP (*crtbegin.o(.ctors))
      KEEP (*(EXCLUDE_FILE (*crtend.o) .ctors))
      KEEP (*(SORT(.ctors.*)))
      KEEP (*crtend.o(.ctors))
      _ctor_end = .;
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
}
