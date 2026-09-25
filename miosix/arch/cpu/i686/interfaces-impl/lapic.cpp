// lapic.cpp
#include "lapic.h"

namespace miosix {
#define IA32_APIC_BASE_MSR 0x1B
#define IA32_APIC_BASE_MSR_BSP 0x100 // Processor is a BSP
#define IA32_APIC_BASE_MSR_ENABLE 0x800
#define APIC_LVT_TIMER_MODE_PERIODIC 0x20000
#define APIC_LVT_INT_MASKED 0x10000

// Address of the LAPIC
static volatile localAPICStruct *lapic = nullptr;

static inline uint64_t rdmsr(uint32_t msr) {
  uint32_t lo;
  uint32_t hi;

  asm volatile("rdmsr" : "=a"(lo), "=d"(hi) : "c"(msr));

  return ((uint64_t)hi << 32) | lo;
}

static inline void wrmsr(uint32_t msr, uint64_t value) {
  uint32_t lo = (uint32_t)value;
  uint32_t hi = (uint32_t)(value >> 32);

  asm volatile("wrmsr" : : "c"(msr), "a"(lo), "d"(hi) : "memory");
}

// Helper functions for pit and pic
static inline uint8_t inb(uint16_t port) {
  uint8_t ret;
  asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
  return ret;
}
static inline void outb(uint16_t port, uint8_t val) {
  asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static void PITPrepareSleep(uint32_t microseconds) {
  // since the PIT has a fixed frequency we can calculate the number of PIT
  // ticks we need to sleep
  uint32_t ticks = (microseconds * 1193) / 1000;

  // Set the Timer 2 on Mode 0 with interrupt on terminal count
  outb(0x43, 0xB0);
  // Send the number of ticks in to step since we can send 16 bits at time
  outb(0x42, ticks & 0xFF);
  outb(0x42, (ticks >> 8) & 0xFF);
}

static void PITPerformSleep() {
  // Read on PC speaker
  uint8_t port61 = inb(0x61);
  // initialize PIT Ch 2 in one-shot mode
  //  Turn off the speaker to not emit sound
  outb(0x61, (port61 & 0xFD) | 0x01);

  // wait for the countdown
  while ((inb(0x61) & 0x20) == 0) {
    // Busy wait
    asm volatile("pause");
  }

  // shtudown channel 2
  outb(0x61, port61 & 0xFC);
}

void initLAPIC() {
  // Turn off PIC
  outb(0x21, 0xFF); // Master
  outb(0xA1, 0xFF); // Slave

  // Activate APIC
  uint64_t apic_msr = rdmsr(IA32_APIC_BASE_MSR);
  apic_msr |= IA32_APIC_BASE_MSR_ENABLE;
  wrmsr(IA32_APIC_BASE_MSR, apic_msr);

  // Get apic base address
  // #TODO: Remember to add it to virtual memory; For correct operation the
  // local APIC registers should be mapped as 'strong uncachable'.
  lapic = (localAPICStruct *)(apic_msr & 0xFFFFF000);
  // The Task Priority Register (TPR) is a 32-bit register used to control the
  // minimum priority an interrupt needs to have for it to be delivered. 0 is
  // lowest, 15 is highest and only higher AND NOT higher or equal is delivered
  lapic->tpr.reg = 0;

  // Set the Spurious Interrupt Vector Register bit 8 to start receiving
  // interrupts
  lapic->sivr.reg = 0x100 | 0xFF;
}

void sendEOI() {
  // Write 0 in the EOI registers to let the APIC resume sending interrupts to
  // the processors
  lapic->eoi.reg = 0;
}

void initLAPICTimer(uint32_t timer_interrupt_vector) {

  if (!lapic)
    return;
  asm volatile("cli");
  // Tell APIC timer to use divider 16
  lapic->timer_divide.reg = 0x03;

  // Prepare the PIT to sleep for 10ms (10000µs)
  PITPrepareSleep(10000);
  //
  // Set APIC init counter to -1
  lapic->timer_initial.reg = 0xFFFFFFFF;
  //
  // Perform PIT-supported sleep
  PITPerformSleep();
  // Stop the APIC timer
  lapic->lvt_timer.reg = APIC_LVT_INT_MASKED;
  // Now we know how often the APIC timer has ticked in 10ms
  uint32_t ticksIn10ms = 0xFFFFFFFF - lapic->timer_current.reg;

  uint32_t ticks_1ms = ticksIn10ms / 10;

  // Start timer as periodic on requested vector, divider 16, with 1ms ticks

  lapic->lvt_timer.reg = timer_interrupt_vector | APIC_LVT_TIMER_MODE_PERIODIC;

  lapic->timer_divide.reg = 0x03;
  lapic->timer_initial.reg = ticks_1ms;
}

} // namespace miosix
