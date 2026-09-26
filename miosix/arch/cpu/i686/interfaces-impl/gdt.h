#pragma once
#include "stdint.h"

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
  uint16_t limit;
  uint16_t base_low;
  uint8_t base_middle;
  uint8_t access;
  uint8_t flags;
  uint8_t base_high;
} __attribute__((
    packed)); // used to tell the compiler to use as little memory as possible

struct GDTPointerStruct {
  uint16_t limit;
  uint32_t base;
} __attribute__((packed));

struct TSSEntryStruct {
  uint32_t prev_tss;
  uint32_t esp0;
  uint32_t ss0;
  uint32_t esp1;
  uint32_t ss1;
  uint32_t esp2;
  uint32_t ss2;
  uint32_t cr3;
  uint32_t eip;
  uint32_t eflags;
  uint32_t eax;
  uint32_t ecx;
  uint32_t edx;
  uint32_t ebx;
  uint32_t esp;
  uint32_t ebp;
  uint32_t esi;
  uint32_t edi;
  uint32_t es;
  uint32_t cs;
  uint32_t ss;
  uint32_t ds;
  uint32_t fs;
  uint32_t gs;
  uint32_t ldt;
  uint32_t trap;
  uint32_t iomap_base;
} __attribute__((packed));

void initGDT();
void setGDTGate(
    uint32_t num, uint32_t base, uint32_t limit, uint8_t access,
    uint8_t gran); // altough limit is 20 bits, we don't have a uint20_t
void writeTSS(uint32_t num, uint16_t ss0, uint32_t esp0);
