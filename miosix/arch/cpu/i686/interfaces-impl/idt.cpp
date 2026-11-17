#pragma once
#include "idt.h"
#include "interfaces/poweroff.h"
#include "interrupts.h"
#include "kernel/logging.h"
#include "lapic.h"
#include "string.h"
#include "util/util.h"
#include "vga.h"
#include <cstdint>

namespace miosix {

struct IDTEntryStruct IDTEntries[256];
struct IDTPointerStruct IDTPointer;

extern "C" void loadIDT(uint32_t);

#ifdef WITH_ERRLOG

static void printUnsignedInt(unsigned int x) {
  char result[] = "0x........\r\n";
  formatHex(result + 2, x, 8);
  IRQerrorLog(result);
}

static void tryPrintingProgramCounter(unsigned int eip, unsigned int esp) {
  // Miosix uses the stack-in-heap approach for kernel threads, so compare the
  // stack pointer against the boundaries of the heap
  extern char _end asm("_end");           // defined in the linker script
  extern char _heap_end asm("_heap_end"); // defined in the linker script
  const unsigned int off = 4; // Offset in bytes of PC in the stack frame on x86
  if (esp < reinterpret_cast<unsigned int>(&_end) ||
      esp > reinterpret_cast<unsigned int>(&_heap_end) - off -
                sizeof(unsigned int))
    IRQerrorLog("? (Corrupted stack)\r\n");
  else
    printUnsignedInt(eip);
}

#endif // WITH_ERRLOG
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

  setIDTGate(1, (uint32_t)isr1, 0x08, 0x8E);
  setIDTGate(2, (uint32_t)isr2, 0x08, 0x8E);
  setIDTGate(3, (uint32_t)isr3, 0x08, 0x8E);
  setIDTGate(4, (uint32_t)isr4, 0x08, 0x8E);
  setIDTGate(5, (uint32_t)isr5, 0x08, 0x8E);
  setIDTGate(6, (uint32_t)isr6, 0x08, 0x8E);
  setIDTGate(7, (uint32_t)isr7, 0x08, 0x8E);
  setIDTGate(8, (uint32_t)isr8, 0x08, 0x8E);
  setIDTGate(9, (uint32_t)isr9, 0x08, 0x8E);
  setIDTGate(10, (uint32_t)isr10, 0x08, 0x8E);
  setIDTGate(11, (uint32_t)isr11, 0x08, 0x8E);
  setIDTGate(12, (uint32_t)isr12, 0x08, 0x8E);
  setIDTGate(13, (uint32_t)isr13, 0x08, 0x8E);
  setIDTGate(14, (uint32_t)isr14, 0x08, 0x8E);
  setIDTGate(15, (uint32_t)isr15, 0x08, 0x8E);
  setIDTGate(16, (uint32_t)isr16, 0x08, 0x8E);
  setIDTGate(17, (uint32_t)isr17, 0x08, 0x8E);
  setIDTGate(18, (uint32_t)isr18, 0x08, 0x8E);
  setIDTGate(19, (uint32_t)isr19, 0x08, 0x8E);
  setIDTGate(20, (uint32_t)isr20, 0x08, 0x8E);
  setIDTGate(21, (uint32_t)isr21, 0x08, 0x8E);
  setIDTGate(22, (uint32_t)isr22, 0x08, 0x8E);
  setIDTGate(23, (uint32_t)isr23, 0x08, 0x8E);
  setIDTGate(24, (uint32_t)isr24, 0x08, 0x8E);
  setIDTGate(25, (uint32_t)isr25, 0x08, 0x8E);
  setIDTGate(26, (uint32_t)isr26, 0x08, 0x8E);
  setIDTGate(27, (uint32_t)isr27, 0x08, 0x8E);
  setIDTGate(28, (uint32_t)isr28, 0x08, 0x8E);
  setIDTGate(29, (uint32_t)isr29, 0x08, 0x8E);
  setIDTGate(30, (uint32_t)isr30, 0x08, 0x8E);
  setIDTGate(31, (uint32_t)isr31, 0x08, 0x8E);

  setIDTGate(32, (uint32_t)irq0, 0x08, 0x8E);
  setIDTGate(33, (uint32_t)irq1, 0x08, 0x8E);
  setIDTGate(34, (uint32_t)irq2, 0x08, 0x8E);
  setIDTGate(35, (uint32_t)irq3, 0x08, 0x8E);
  setIDTGate(36, (uint32_t)irq4, 0x08, 0x8E);
  setIDTGate(37, (uint32_t)irq5, 0x08, 0x8E);
  setIDTGate(38, (uint32_t)irq6, 0x08, 0x8E);
  setIDTGate(39, (uint32_t)isr_spurious_master, 0x08, 0x8E);
  setIDTGate(40, (uint32_t)irq8, 0x08, 0x8E);
  setIDTGate(41, (uint32_t)irq9, 0x08, 0x8E);
  setIDTGate(42, (uint32_t)irq10, 0x08, 0x8E);
  setIDTGate(43, (uint32_t)irq11, 0x08, 0x8E);
  setIDTGate(44, (uint32_t)irq12, 0x08, 0x8E);
  setIDTGate(45, (uint32_t)irq13, 0x08, 0x8E);
  setIDTGate(46, (uint32_t)irq14, 0x08, 0x8E);
  setIDTGate(47, (uint32_t)isr_spurious_slave, 0x08, 0x8E);

  // interrupt for calling the scheduler
  setIDTGate(48, (uint32_t)PendSV_Handler, 0x08, 0x8E);
  setIDTGate(49, (uint32_t)irq17, 0x08, 0x8E);

  loadIDT((uint32_t)&IDTPointer); // Loads the IDT and fills it with the CPU
                                  // excpetions and IRQs
  const char buf[] = "IDT done\r\n";
  for (size_t i = 0; i < 10; i++)
    print(buf[i]);
}

void setIDTGate(uint8_t num, uint32_t offset, uint16_t selector,
                uint8_t flags) {
  IDTEntries[num].offset_low = offset & 0xFFFF;
  IDTEntries[num].offset_high = (offset >> 16) & 0xFFFF;
  IDTEntries[num].selector = selector;
  IDTEntries[num].zero = 0;

  IDTEntries[num].flags = flags | 0x60;
}

extern "C" {

const char *isrMessages[32] = {"Division Error\r\n",
                               "Debug\r\n",
                               "Non-maskable Interrupt\r\n",
                               "Breakpoint\r\n",
                               "Overflow\r\n",
                               "Bound Range Exceeded\r\n",
                               "Invalid Opcode\r\n",
                               "Device Not Available\r\n",
                               "Double Fault\r\n",
                               "Coprocessor Segment Overrun\r\n",
                               "Invalid TSS\r\n",
                               "Segment Not Present\r\n",
                               "Stack-Segment Fault\r\n",
                               "General Protection Fault\r\n",
                               "Page Fault\r\n",
                               "Reserved\r\n",
                               "x87 Floating-Point Exception\r\n",
                               "Alignment Check\r\n",
                               "Machine Check\r\n",
                               "SIMD Floating-Point Exception\r\n",
                               "Virtualization Exception\r\n",
                               "Control Protection Exception\r\n",
                               "Reserved\r\n",
                               "Reserved\r\n",
                               "Reserved\r\n",
                               "Reserved\r\n",
                               "Reserved\r\n",
                               "Reserved\r\n",
                               "Hypervisor Injection Exception\r\n",
                               "VMM Communication Exception\r\n",
                               "Security Exception\r\n",
                               "Reserved\r\n"};
void isrHandler(struct InterruptRegisters *regs) {
  FastGlobalLockFromIrq lock;

#ifdef WITH_ERRLOG
  IRQerrorLog("\r\n***Unexpected HardFault @ ");
  tryPrintingProgramCounter(regs->eip, regs->esp);
  IRQerrorLog(isrMessages[regs->int_no]);
#endif // WITH_ERRLOG
  for (;;)
    asm volatile("cli; hlt");
  IRQsystemReboot();
}
}

} // namespace miosix
