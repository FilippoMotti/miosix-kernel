// lapic.h
#pragma once
#include "stdint.h"

namespace miosix {

// The local APIC registers are memory mapped to an address that can be found in
// the MP/MADT tables. Make sure you map these to virtual memory if you are
// using paging. Each register is 32 bits long, and expects to be written and
// read as a 32 bit integer. Although each register is 4 bytes, they are all
// aligned on a 16 byte boundary.
// This is important since the icr is 64 bit long, so it has to be divided in 2

// did this to avoid having every reg followed by padding in the big struct
typedef struct {
  volatile uint32_t reg;
  uint32_t padding[3];
} apic_reg_t;

// DO NOT modify this order, as this mimic the actual order of the entry
typedef struct {
  apic_reg_t reserved0[2];  // 0x000 - 0x010
  apic_reg_t id;            // 0x020 - Local APIC ID
  apic_reg_t version;       // 0x030 - Local APIC Version
  apic_reg_t reserved1[4];  // 0x040 - 0x070
  apic_reg_t tpr;           // 0x080 - Task Priority Register
  apic_reg_t apr;           // 0x090 - Arbitration Priority Register
  apic_reg_t ppr;           // 0x0A0 - Processor Priority Register
  apic_reg_t eoi;           // 0x0B0 - End of Interrupt Register
  apic_reg_t rrd;           // 0x0C0 - Remote Read Register
  apic_reg_t ldr;           // 0x0D0 - Logical Destination Register
  apic_reg_t dfr;           // 0x0E0 - Destination Format Register
  apic_reg_t sivr;          // 0x0F0 - Spurious Interrupt Vector Register
  apic_reg_t isr[8];        // 0x100 - 0x170: In-Service Register
  apic_reg_t tmr[8];        // 0x180 - 0x1F0: Trigger Mode Register
  apic_reg_t irr[8];        // 0x200 - 0x270: Interrupt Request Register
  apic_reg_t error_status;  // 0x280 - Error Status Register
  apic_reg_t reserved2[6];  // 0x290 - 0x2E0
  apic_reg_t lvt_cmci;      // 0x2F0 - LVT Corrected Machine Check Interrupt
  apic_reg_t icr_low;       // 0x300 - Interrupt Command Register [0:31]
  apic_reg_t icr_high;      // 0x310 - Interrupt Command Register [32:63]
  apic_reg_t lvt_timer;     // 0x320 - LVT Timer Register
  apic_reg_t lvt_thermal;   // 0x330 - LVT Thermal Sensor Register
  apic_reg_t lvt_perf;      // 0x340 - LVT Performance Monitor Register
  apic_reg_t lvt_lint0;     // 0x350 - LVT LINT0 Register
  apic_reg_t lvt_lint1;     // 0x360 - LVT LINT1 Register
  apic_reg_t lvt_error;     // 0x370 - LVT Error Register
  apic_reg_t timer_initial; // 0x380 - Initial Count Register (for timer)
  apic_reg_t timer_current; // 0x390 - Current Count Register (for timer)
  apic_reg_t reserved3[4];  // 0x3A0 - 0x3D0
  apic_reg_t timer_divide;  // 0x3E0 - Timer Divide Configuration Register
  apic_reg_t reserved4;     // 0x3F0 - Reserved
} __attribute__((packed)) localAPICStruct;

// Initialize local apic and diables pic
void initLAPIC();

void sendEOI();

uint32_t getIdCore();

void initLAPICTimer(uint32_t timer_interrupt_vector);

} // namespace miosix
