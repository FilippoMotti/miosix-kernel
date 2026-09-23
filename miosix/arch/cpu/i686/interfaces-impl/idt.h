#pragma once
#include "stdint.h"

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

void isrHandler(struct InterruptRegisters *regs);
void irqInstallHandler(int irq, void (*handler)(struct InterruptRegisters *r));
void irqUninstallHandler(int irq);
void irqHandler(struct InterruptRegisters *regs);

extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();

extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();
