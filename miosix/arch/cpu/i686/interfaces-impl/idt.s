global loadIDT
loadIDT:
    MOV eax, [esp+4]
    LIDT [eax]
    STI
    RET
%macro ISR_NOERRCODE 1
    global isr%1
    isr%1:
        CLI
        PUSH LONG 0
        PUSH LONG %1
        JMP isrCommmonStub
%endmacro

%macro ISR_ERRCODE 1
    global isr%1
    isr%1:
        CLI
        PUSH LONG %1
        JMP isrCommmonStub
%endmacro

%macro IRQ 2
    global irq%1
    irq%1:
        CLI
        PUSH LONG 0
        PUSH LONG %2
        JMP irqCommonStub
%endmacro

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
ISR_NOERRCODE 9     # Coprocessor Segment Overrun (outdated)
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
IRQ   0,    32 # Programmable Interrupt Timer Interrupt
IRQ   1,    33 # Keyboard Interrupt
IRQ   2,    34 # Cascade (used internally by the two PICs. never raised)
IRQ   3,    35 # COM2 (if enabled)
IRQ   4,    36 # COM1 (if enabled)
IRQ   5,    37 # LPT2 (if enabled)
IRQ   6,    38 # Floppy Disk
IRQ   7,    39 # LPT1 / Unreliable "spurious" interrupt (usually)
IRQ   8,    40 # CMOS real-time clock (if enabled)
IRQ   9,    41 # Free for peripherals / legacy SCSI / NIC
IRQ  10,    42 # Free for peripherals / SCSI / NIC
IRQ  11,    43 # Free for peripherals / SCSI / NIC
IRQ  12,    44 # PS2 Mouse
IRQ  13,    45 # FPU / Coprocessor / Inter-processor
IRQ  14,    46 # Primary ATA Hard Disk
IRQ  15,    47 # Secondary ATA Hard Disk

extern isrHandler
isrCommmonStub:
#Saving all registers
    pusha
    mov eax,ds
    PUSH eax
    MOV eax, cr2
    PUSH eax

#load data segment in gdt (kernel data segment is in 0x10)
    MOV ax, 0x10
    MOV ds, ax
    MOV es, ax
    MOV fs, ax
    MOV gs, ax

#pass the stack pointer to the function, so that it can pop all registers and put them in the struct
    PUSH esp
    CALL isrHandler

#Restore original state
    ADD esp, 8
    POP ebx
    MOV ds, bx
    MOV es, bx
    MOV fs, bx
    MOV gs, bx

    POPA
    ADD esp, 8
    STI
    IRET

extern irqHandler
irqCommonStub:
    pusha
    mov eax,ds
    PUSH eax
    MOV eax, cr2
    PUSH eax

    MOV ax, 0x10
    MOV ds, ax
    MOV es, ax
    MOV fs, ax
    MOV gs, ax

    PUSH esp
    CALL irqHandler

    ADD esp, 8
    POP ebx
    MOV ds, bx
    MOV es, bx
    MOV fs, bx
    MOV gs, bx

    POPA
    ADD esp, 8
    STI
    IRET
