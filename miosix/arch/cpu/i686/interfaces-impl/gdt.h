#pragma once
#include "stdint.h"
namespace miosix {

// DO NOT modify this order, as this mimic the actual order of the entry
//------------------------------------------------------|
//|63       56|55   52|51   48|47         40|39       32|
//|-----------|-------|-------|-------------|-----------|
//|Base       |Flags  |Limit  |Access Bytes |Base       |
//|           |       |       |             |           |
//|31       24|3    0 |19   16|7           0|23       16|
//|-----------------------------------------------------|
//|31                       16|15                      0|
//|---------------------------|-------------------------|
//|Base                       |Limit                    |
//|                           |                         |
//|15                        0|15                      0|
//-------------------------------------------------------
//
struct GDTEntryStruct {
  volatile uint16_t limit;
  volatile uint16_t base_low;
  volatile uint8_t base_middle;
  volatile uint8_t access;
  volatile uint8_t flags;
  volatile uint8_t base_high;
} __attribute__((
    packed)); // used to tell the compiler to use as little memory as possible

struct GDTPointerStruct {
  volatile uint16_t limit;
  volatile uint32_t base;
} __attribute__((packed));

struct TSSEntryStruct {
  volatile uint32_t prev_tss;
  volatile uint32_t esp0;
  volatile uint32_t ss0;
  volatile uint32_t esp1;
  volatile uint32_t ss1;
  volatile uint32_t esp2;
  volatile uint32_t ss2;
  volatile uint32_t cr3;
  volatile uint32_t eip;
  volatile uint32_t eflags;
  volatile uint32_t eax;
  volatile uint32_t ecx;
  volatile uint32_t edx;
  volatile uint32_t ebx;
  volatile uint32_t esp;
  volatile uint32_t ebp;
  volatile uint32_t esi;
  volatile uint32_t edi;
  volatile uint32_t es;
  volatile uint32_t cs;
  volatile uint32_t ss;
  volatile uint32_t ds;
  volatile uint32_t fs;
  volatile uint32_t gs;
  volatile uint32_t ldt;
  volatile uint32_t trap;
  volatile uint32_t iomap_base;
} __attribute__((packed));

void initGDT();
void setGDTGate(
    uint32_t num, uint32_t base, uint32_t limit, uint8_t access,
    uint8_t gran); // altough limit is 20 bits, we don't have a uint20_t
void writeTSS(uint32_t num, uint16_t ss0, uint32_t esp0);
} // namespace miosix
