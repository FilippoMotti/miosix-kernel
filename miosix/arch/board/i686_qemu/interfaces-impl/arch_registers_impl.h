#pragma once

// On x86 we don't have RCC, but the hardware itself guaranties that we
// have correct order, so we just use a software fence
#define RCC_SYNC() asm volatile("" ::: "memory")

// We use APIC that let us aving 256 slots, but the first 32 are reserved
// to hardware interrupts, we have 224 irqs.
#define MIOSIX_NUM_PERIPHERAL_IRQ 224
