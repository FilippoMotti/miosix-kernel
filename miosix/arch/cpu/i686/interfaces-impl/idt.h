#pragma once
#include "stdint.h"
namespace miosix {

// DO NOT modify this order, as this mimic the actual order of the entry
//--------------------------------------------------------------|
//|63                       48|47|46 45|44|43     40|39       32|
//|---------------------------|--|-----|--|---------|-----------|
//|Offset                     |P |DPL  |0 |Gate Type|Reserved   |
//|                           |  |     |  |         |           |
//|31                       16|  |1   0|  |3       0|           |
//|-------------------------------------------------------------|
//|31                       16|15                              0|
//|---------------------------|---------------------------------|
//|Segment Selector           |Offset                           |
//|                           |                                 |
//|15                        0|15                              0|
//---------------------------------------------------------------
//
struct IDTEntryStruct {
  uint16_t offset_low;
  uint16_t selector; // code segment selector
  uint8_t zero;
  uint8_t flags; // gate type, dpl, and p fields
  uint16_t offset_high;
} __attribute__((packed));

struct IDTPointerStruct {
  uint16_t limit;
  uint32_t base;
} __attribute__((packed));

typedef enum {
  IDT_FLAG_GATE_TASK = 0x5,
  IDT_FLAG_GATE_16BIT_INT = 0x6,
  IDT_FLAG_GATE_16BIT_TRAP = 0x7,
  IDT_FLAG_GATE_32BIT_INT = 0xE,
  IDT_FLAG_GATE_32BIT_TRAP = 0xF,

  IDT_FLAG_RING0 = (0 << 5),
  IDT_FLAG_RING1 = (1 << 5),
  IDT_FLAG_RING2 = (2 << 5),
  IDT_FLAG_RING3 = (3 << 5),

  IDT_FLAG_PRESENT = 0x80,
} IDT_FLAGS;

struct InterruptRegisters {
  uint32_t cr2;
  uint32_t ds;
  uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
  uint32_t int_no, err_code;
  uint32_t eip, csm, eflags, useresp, ss;
};

void initIDT();
void setIDTGate(uint8_t num, uint32_t offset, uint16_t selector, uint8_t flags);

void irqInstallHandler(int irq, void (*handler)(struct InterruptRegisters *r));
void irqUninstallHandler(int irq);

extern "C" {

void loadIDT(uint32_t);

void isrHandler(struct InterruptRegisters *regs);
void irqHandler(struct InterruptRegisters *regs);

void isr0();
void isr1();
void isr2();
void isr3();
void isr4();
void isr5();
void isr6();
void isr7();
void isr8();
void isr9();
void isr10();
void isr11();
void isr12();
void isr13();
void isr14();
void isr15();
void isr16();
void isr17();
void isr18();
void isr19();
void isr20();
void isr21();
void isr22();
void isr23();
void isr24();
void isr25();
void isr26();
void isr27();
void isr28();
void isr29();
void isr30();
void isr31();

void irq0();
void irq1();
void irq2();
void irq3();
void irq4();
void irq5();
void irq6();
void irq7();
void irq8();
void irq9();
void irq10();
void irq11();
void irq12();
void irq13();
void irq14();
void irq15();
}
} // namespace miosix
