ENTRY(_start)

SECTIONS
{
    . = 2M;
    
    .boot :
    {
        *(.multiboot)
        *(.boot)
    }

    . += 0x80000000;

    .bootstrap_tables ALIGN(4K) : AT(ADDR(.bootstrap_tables) - 0x80000000)
    {
        *(.bootstrap_tables)
    }

    .text ALIGN(4K) : AT(ADDR(.text) - 0x80000000)
    {
        _text = .;
        KEEP(*(.multiboot))
        *(.text)
        *(.text.*)
        _etext = .;
    }

    .rodata ALIGN(4K) : AT(ADDR(.rodata) - 0x80000000)
    {
        *(.rodata)
        *(.rodata.*)
    }

    .eh_frame_hdr ALIGN(4K) : AT(ADDR(.eh_frame_hdr) - 0x80000000)
    {
        _EH_FRAME_HDR = .;
        *(.eh_frame_hdr)
        _EH_FRAME_HDR_END = .;
    }
    
    .eh_frame ALIGN(4K) : AT(ADDR(.eh_frame) - 0x80000000)
    {
        KEEP(*(.eh_frame))
        KEEP(*(.eh_frame.*))
    }

    .gcc_except_table ALIGN(4K) : AT(ADDR(.gcc_except_table) - 0x80000000)
    {
        *(.gcc_except_table)
        *(.gcc_except_table.*)
    }

    .data ALIGN(4K) : AT(ADDR(.data) - 0x80000000)
    {
        _data = .;
        *(.data)
        *(.data.*)
        _edata = .;
    }

    .init_array ALIGN(4K) : AT(ADDR(.init_array) - 0x80000000)
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

    .bss ALIGN(4K) : AT(ADDR(.bss) - 0x80000000)
    {
        _bss_start = .;
        *(COMMON)
        *(.bss)
        *(.bss.*)
        _bss_end = .;
    }

    .bootstrap_stack ALIGN(4K) : AT(ADDR(.bootstrap_stack) - 0x80000000)
    {
        *(.bootstrap_stack)
    }
    .page_pool (NOLOAD) : ALIGN(4K)
    {
        _page_pool_start = .;
        *(.page_pool)
        _page_pool_end = .;
    }

    _end = .;
    _heap_end = _end + 16M;
}
