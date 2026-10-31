.set MAGIC,    0xE85250D6       /* 'magic number' lets bootloader find the header */
.set CHECKSUM, -(MAGIC+ 0 + (header_end - header_start)) /* checksum of above, to prove we are multiboot */

.set KERNEL_VIRTUAL_BASE, 0x80000000
.set KERNEL_PAGE_NUMBER,  (KERNEL_VIRTUAL_BASE >> 22) # 512

.section .multiboot, "aw"
.align 8
header_start:
    .long MAGIC
    .long 0                         # Architecture 0 (protected mode i386)
    .long header_end - header_start # Header length
    .long CHECKSUM

    .align 8
    .short 0                        # Type = 0 (End tag)
    .short 0                        # Flags = 0
    .long 8                         # Size = 8 byte
header_end:


.section .bootstrap_stack, "aw", @nobits
.align 16
stack_bottom:
.skip 16384 # 16 KiB
stack_top:

.section .boot, "a"
.global _start
.type _start, @function
_start:
    movl    $(initial_page_dir - KERNEL_VIRTUAL_BASE), %ecx
    movl    %ecx, %cr3

    movl    %cr4, %ecx
    orl     $0x10, %ecx
    movl    %ecx, %cr4

    movl    %cr0, %ecx
    orl     $0x80000000, %ecx
    movl    %ecx, %cr0

    lea     higher_half, %ecx
    jmp     *%ecx

.section .text
higher_half:
    cli
    mov     $stack_top, %esp
    pushl   %ebx
    pushl   %eax
    xor     %ebp, %ebp

    call    kernel_main

    cli
1:  hlt
    jmp     1b

.section .data
.balign 4096
.globl initial_page_dir
initial_page_dir:
    .long 0b10000011

    .rept 512 - 1
    .long 0
    .endr

    .long (0 << 22) | 0b10000011
    .long (1 << 22) | 0b10000011
    .long (2 << 22) | 0b10000011
    .long (3 << 22) | 0b10000011

    .rept 1024 - 512 - 4
    .long 0
    .endr
