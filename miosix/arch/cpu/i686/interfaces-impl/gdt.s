global GDTLoad

GDTLoad:
    MOV eax, [esp+4]
    LGDT [eax]

    MOV eax, 0x10 #Update segment registers
    MOV ds, ax
    MOV es, ax
    MOV fs, ax
    MOV gs, ax
    MOV ss, ax
    JMP 0x08:.flush # Update Code Segment by doing a far jump
.flush:
    RET

global TSSLoad

TSSLoad:
  MOV ax, 0x28
  LTR ax



