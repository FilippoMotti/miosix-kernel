#pragma once
#include "idt.h"

namespace miosix {

struct IDTEntryStruct IDTEntries[256];
struct IDTPointerStruct IDTPointer;

extern void loadIDT(uint32_t);

void initIDT() {
  // disabling interrupts when loading the new IDT
  asm volatile("cli");
  IDTPointer.limit = (sizeof(struct IDTEntryStruct) * 256) - 1;
  IDTPointer.base = reinterpret_cast<uint32_t>(&IDTEntries);

  memset(&IDTEntries, 0, sizeof(IDTEntryStruct) * 256); // Zeroing all the bytes

  setIDTGate(0, (uint32_t)isr0, 0x08, 0x8E);
  // 0x8E = 1000 1110
  // from left to right: 1st bit: P bit (must be 1)
  // 2nd and 3rd bit: DPL
  // 4th bit is always 0
  // last 4 bits are the Gate Type: 0xE means 32 bit Interrupt Gate
  //
  // 0x08 represent the code segment defined in the GDT (in this case as Kernel
  // Code Segment)

  setIdtGate(1, (uint32_t)isr1, 0x08, 0x8E);
  setIdtGate(2, (uint32_t)isr2, 0x08, 0x8E);
  setIdtGate(3, (uint32_t)isr3, 0x08, 0x8E);
  setIdtGate(4, (uint32_t)isr4, 0x08, 0x8E);
  setIdtGate(5, (uint32_t)isr5, 0x08, 0x8E);
  setIdtGate(6, (uint32_t)isr6, 0x08, 0x8E);
  setIdtGate(7, (uint32_t)isr7, 0x08, 0x8E);
  setIdtGate(8, (uint32_t)isr8, 0x08, 0x8E);
  setIdtGate(9, (uint32_t)isr9, 0x08, 0x8E);
  setIdtGate(10, (uint32_t)isr10, 0x08, 0x8E);
  setIdtGate(11, (uint32_t)isr11, 0x08, 0x8E);
  setIdtGate(12, (uint32_t)isr12, 0x08, 0x8E);
  setIdtGate(13, (uint32_t)isr13, 0x08, 0x8E);
  setIdtGate(14, (uint32_t)isr14, 0x08, 0x8E);
  setIdtGate(15, (uint32_t)isr15, 0x08, 0x8E);
  setIdtGate(16, (uint32_t)isr16, 0x08, 0x8E);
  setIdtGate(17, (uint32_t)isr17, 0x08, 0x8E);
  setIdtGate(18, (uint32_t)isr18, 0x08, 0x8E);
  setIdtGate(19, (uint32_t)isr19, 0x08, 0x8E);
  setIdtGate(20, (uint32_t)isr20, 0x08, 0x8E);
  setIdtGate(21, (uint32_t)isr21, 0x08, 0x8E);
  setIdtGate(22, (uint32_t)isr22, 0x08, 0x8E);
  setIdtGate(23, (uint32_t)isr23, 0x08, 0x8E);
  setIdtGate(24, (uint32_t)isr24, 0x08, 0x8E);
  setIdtGate(25, (uint32_t)isr25, 0x08, 0x8E);
  setIdtGate(26, (uint32_t)isr26, 0x08, 0x8E);
  setIdtGate(27, (uint32_t)isr27, 0x08, 0x8E);
  setIdtGate(28, (uint32_t)isr28, 0x08, 0x8E);
  setIdtGate(29, (uint32_t)isr29, 0x08, 0x8E);
  setIdtGate(30, (uint32_t)isr30, 0x08, 0x8E);
  setIdtGate(31, (uint32_t)isr31, 0x08, 0x8E);

  setIdtGate(32, (uint32_t)irq0, 0x08, 0x8E);
  setIdtGate(33, (uint32_t)irq1, 0x08, 0x8E);
  setIdtGate(34, (uint32_t)irq2, 0x08, 0x8E);
  setIdtGate(35, (uint32_t)irq3, 0x08, 0x8E);
  setIdtGate(36, (uint32_t)irq4, 0x08, 0x8E);
  setIdtGate(37, (uint32_t)irq5, 0x08, 0x8E);
  setIdtGate(38, (uint32_t)irq6, 0x08, 0x8E);
  setIdtGate(39, (uint32_t)irq7, 0x08, 0x8E);
  setIdtGate(40, (uint32_t)irq8, 0x08, 0x8E);
  setIdtGate(41, (uint32_t)irq9, 0x08, 0x8E);
  setIdtGate(42, (uint32_t)irq10, 0x08, 0x8E);
  setIdtGate(43, (uint32_t)irq11, 0x08, 0x8E);
  setIdtGate(44, (uint32_t)irq12, 0x08, 0x8E);
  setIdtGate(45, (uint32_t)irq13, 0x08, 0x8E);
  setIdtGate(46, (uint32_t)irq14, 0x08, 0x8E);
  setIdtGate(47, (uint32_t)irq15, 0x08, 0x8E);

  loadIDT((uint32_t)&IDTPointer); // Loads the IDT and fills it with the CPU
                                  // excpetions and IRQs
}

void setIDTGate(uint8_t num, uint32_t offset, uint16_t selector,
                uint8_t flags) {
  IDTEntries[num].offset_low = offset & 0xFFFF;
  IDTEntries[num].offset_high = (offset >> 16) & 0xFFFF;
  IDTEntries[num].selector = selector;
  IDTEntries[num].zero = 0;

  IDTEntries[num].flags = flags | 0x60;
}

// #TODO: implement the actual handler. TBD
void isrHandler(struct InterruptRegisters *regs) {
  if (regs->int_no < 32) {
  }
}

void *irqRoutines[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void irqInstallHandler(int irq, void (*handler)(struct InterruptRegisters *r)) {
  irqRoutines[irq] = handler;
}

void irqUninstallHandler(int irq) { irqRoutines[irq] = 0; }

} // namespace miosix
