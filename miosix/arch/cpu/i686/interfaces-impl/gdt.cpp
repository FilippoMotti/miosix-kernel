#pragma once
#include "gdt.h"
#include "string.h"

namespace miosix {

extern void GDTLoad(uint32_t);
extern void TSSLoad();

struct GDTEntryStruct GDTEntries[6];
struct GDTPointerStruct GDTPointer;
struct TSSEntryStruct TSSEntry;

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

void writeTSS(uint32_t num, uint16_t ss0, uint32_t esp0) {
  uint32_t base = (uint32_t)&TSSEntry;
  uint32_t limit = sizeof(TSSEntryStruct) - 1;

  setGDTGate(num, base, limit, 0x89, 0x40);
  memset(&GDTEntries, 0, sizeof(GDTEntries));

  TSSEntry.ss0 = ss0;
  TSSEntry.esp0 = esp0;
  TSSEntry.cs = 0x08;
  TSSEntry.iomap_base = sizeof(TSSEntryStruct);
  TSSLoad();
}

void initGDT() {
  // sub 1 is to use the right offset for memory
  GDTPointer.limit = (sizeof(struct GDTEntryStruct) * 6) - 1;
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

  writeTSS(5, 0x10, 0x0);

  GDTLoad((uint32_t)&GDTPointer);
}

} // namespace miosix
