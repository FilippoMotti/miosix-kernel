#include "gdt.h"

namespace miosix {

struct GDTEntryStruct GDTEntries[5];
struct GDTPointerStruct GDTPointer;

void setGDTGate(uint32_t num, uint32_t base, uint32_t limit, uint8_t access,
                uint8_t gran) {
  GDTEntries[num].base_low = (base & 0xFFFF);
  GDTEntries[num].base_middle = (base >> 16) & 0xFFFF;
  GDTEntries[num].base_high = (base >> 24) & 0xFFFF;

  // those are needed because we need to extract limit and flags, that are both
  // 4 bit united in a uint8_t
  GDTEntries[num].limit = (limit & 0xFFFF);
  GDTEntries[num].flags = (limit >> 16) & 0x0F;
  GDTEntries[num].flags |= (gran & 0xF0);

  GDTEntries[num].access = access;
}

void initGDT() {
  GDTPointer.limit = (sizeof(struct GDTEntryStruct) * 5) -
                     1; // sub 1 is to use the right offset for memory
  GDTPointer.base = reinterpret_cast<uint32_t>(&GDTEntries);

  setGDTGate(0, 0, 0, 0, 0);              // Null segment
  setGDTGate(1, 0, 0xFFFFFF, 0x9A, 0xCF); // Kernel Code segment

  // 9A = 1001 1010;
  // 1st bit (from left) is the present bit, 2nd and 3rd are the DPL (0 for
  // kernel segment), 4th is the Descriptor bit (1 for code or data) 5th bit is
  // Executable bit (1 for a code segment that can be executed), 6th is the
  // Direction bit (0 if the segment grows up), 7th is for RW (1 is allowed) 8th
  // bit is Accessed bit (0 because it is not already accesssed)
  setGDTGate(2, 0, 0xFFFFFF, 0x92, 0xCF); // Kernel Data segment
  setGDTGate(3, 0, 0xFFFFFF, 0xFA, 0xCF); // User Code segment
  setGDTGate(4, 0, 0xFFFFFF, 0xF2, 0xCF); // User Code segment

  asm volatile("lgdt [%0]" // Load GDTEntries

               "mov eax, 0x10" // Update segment registers
               "mov ds, ax"
               "mov es, ax"
               "mov fs, ax"
               "mov gs, ax"
               "mov ss, ax"

               "JMP 0x08:.flush" // Update Code Segment by doing a far jump
               ".flush:"
               "RET" ::"r"(&GDTEntries)
               : "eax", "memory");
}

} // namespace miosix
