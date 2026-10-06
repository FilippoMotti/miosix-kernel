.text
.code32

.global GDTLoad
GDTLoad:
    movl 4(%esp), %eax
    lgdt (%eax)

    movw $0x10, %ax #Update segment registers
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss

    ljmp $0x08, $.flush # Update Code Segment by doing a far jump

.flush:
    ret

.global TSSLoad
TSSLoad:
    movw $0x28, %ax
    ltr %ax
    ret


