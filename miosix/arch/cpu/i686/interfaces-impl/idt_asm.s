.text
.code32

.global loadIDT
loadIDT:
    movl 4(%esp), %eax
    lidt (%eax)
    sti
    ret

.macro ISR_NOERRCODE num
    .global isr\num
    isr\num:
        cli
        pushl $0
        pushl $\num
        jmp isrCommonStub
.endm

.macro ISR_ERRCODE num
    .global isr\num
    isr\num:
        cli
        pushl $\num
        jmp isrCommonStub
.endm

.macro IRQ irq_num, vec_num
    .global irq\irq_num
    irq\irq_num:
        cli
        pushl $0
        pushl $\vec_num
        jmp irqCommonStub
.endm

# CPU Exceptions
ISR_NOERRCODE 0     # Division Error
ISR_NOERRCODE 1     # Debug
ISR_NOERRCODE 2     # Non-maskable Interrupt
ISR_NOERRCODE 3     # Breakpoint
ISR_NOERRCODE 4     # Overflow
ISR_NOERRCODE 5     # Bound Range Exceeded
ISR_NOERRCODE 6     # Invalid Opcode
ISR_NOERRCODE 7     # Device Not Available
ISR_ERRCODE   8     # Double Fault
ISR_NOERRCODE 9     # Coprocessor Segment Overrun
ISR_ERRCODE   10    # Invalid TSS
ISR_ERRCODE   11    # Segment Not Present
ISR_ERRCODE   12    # Stack-Segment Fault
ISR_ERRCODE   13    # General Protection Fault
ISR_ERRCODE   14    # Page Fault
ISR_NOERRCODE 15    # Reserved
ISR_NOERRCODE 16    # x87 Floating-Point Exception
ISR_ERRCODE   17    # Alignment Check
ISR_NOERRCODE 18    # Machine Check
ISR_NOERRCODE 19    # SIMD Floating-Point Exception
ISR_NOERRCODE 20    # Virtualization Exception
ISR_NOERRCODE 21    # Control Protection Exception
ISR_NOERRCODE 22    # Reserved
ISR_NOERRCODE 23    # Reserved
ISR_NOERRCODE 24    # Reserved
ISR_NOERRCODE 25    # Reserved
ISR_NOERRCODE 26    # Reserved
ISR_NOERRCODE 27    # Reserved
ISR_NOERRCODE 28    # Hypervisor Injection Exception
ISR_NOERRCODE 29    # VMM Communication Exception
ISR_ERRCODE   30    # Security Exception
ISR_NOERRCODE 31    # Reserved

# Standard ISA IRQ
IRQ 0, 32           # PIT
IRQ 1, 33           # Keyboard
IRQ 2, 34           # Cascade
IRQ 3, 35           # COM2
IRQ 4, 36           # COM1
IRQ 5, 37           # LPT2
IRQ 6, 38           # Floppy Disk
IRQ 7, 39           # LPT1 / Spurious
IRQ 8, 40           # CMOS Real-Time Clock
IRQ 9, 41           # Peripheral
IRQ 10, 42          # Peripheral
IRQ 11, 43          # Peripheral
IRQ 12, 44          # PS/2 Mouse
IRQ 13, 45          # FPU
IRQ 14, 46          # Primary ATA
IRQ 15, 47          # Secondary ATA

.extern isrHandler
isrCommonStub:
    pushal
    movl %ds, %eax
    pushl %eax
    movl %cr2, %eax
    pushl %eax

    #load data segment in gdt (kernel data segment is in 0x10)
    movw $0x10, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs

    #pass the stack pointer to the function, so that it can pop all registers and put them in the struct
    pushl %esp
    call isrHandler

    #Restore original state
    addl $8, %esp
    popl %ebx
    movw %bx, %ds
    movw %bx, %es
    movw %bx, %fs
    movw %bx, %gs

    popal
    addl $8, %esp
    sti
    iretl

.extern irqHandler
irqCommonStub:
    pushal
    movl %ds, %eax
    pushl %eax
    movl %cr2, %eax
    pushl %eax

    movw $0x10, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs

    pushl %esp
    call irqHandler

    addl $8, %esp
    popl %ebx
    movw %bx, %ds
    movw %bx, %es
    movw %bx, %fs
    movw %bx, %gs

    popal
    addl $8, %esp
    sti
    iretl
