#pragma once
#include "stdint.h"

namespace miosix {

// DO NOT modify this order, as this mimic the actual order of the entry
typedef struct {
  volatile uint32_t ioapicreg; // 0x00: Register Select (IORREGSEL)
  uint32_t padding[3];         // Offset 0x04, 0x08, 0x0C
  volatile uint32_t ioapicwin; // 0x10: I/O Window (IOWWIN)
} __attribute__((packed)) IOAPICEntryStruct;

// DO NOT modify this order, as this mimic the actual order of the entry
// the redirection table entry is composed by 2 32-bit registers

// we define the registry as an union of structs, one that explicitly state each
// element and one that is just high and low this definition is helpful when
// reading and writing in the actual register.
union IORedirectionEntry {
  // the number after each name represent the actual size of the section, so
  // that the compiler can pack corretcly the struct in 64 bit
  struct {
    // Lower register

    // The Interrupt vector that will be raised on the specified CPU(s)
    uint32_t vector : 8;

    // How the interrupt will be sent to the CPU(s). It can be 000
    // (Fixed), 001 (Lowest Priority), 010 (SMI), 100 (NMI), 101 (INIT)
    // and 111 (ExtINT). Most of the cases you want Fixed mode, or Lowest
    // Priority if you don't want to suspend a high priority task on some
    // important Processor/Core/Thread.
    uint32_t deliveryMode : 3;

    // Specify how the Destination field shall be interpreted. 0:
    // Physical Destination, 1: Logical Destination
    uint32_t destinationMode : 1;

    // If 0, the IRQ is just relaxed and waiting for something to happen
    // (or it has fired and already processed by Local APIC(s)). If 1, it
    // means that the IRQ has been sent to the Local APICs but it's still
    // waiting to be delivered.
    uint32_t deliveryStatus : 1;

    // 0: Active high, 1: Active low. For ISA IRQs assume Active High
    // unless otherwise specified in Interrupt Source Override
    // descriptors of the MADT or in the MP Tables.
    uint32_t pinPolarity : 1;

    // undefined for edge triggered interrupt. 1 when
    // local APIC(s) accept the level interrupt sent by
    // the IOAPI  0 when an EOI message with a matching
    // interrupt vector is received from a local APIC
    uint32_t remoteIRR : 1;

    // 0: Edge, 1: Level. For ISA IRQs assume Edge unless otherwise
    // specified in Interrupt Source Override descriptors of the MADT or
    // in the MP Tables.
    uint32_t triggerMode : 1;

    // You can temporary disable this IRQ by
    // setting this bit, and reenable it by clearing the bit.
    uint32_t mask : 1;

    uint32_t reserved1 : 15; // reserved

    // higer register

    uint32_t reserved2 : 24;  // reserved
    uint32_t destination : 8; // APIC ID of the cpu that receives the IRQs
  } __attribute__((packed));

  struct {
    uint32_t low;
    uint32_t high;
  };

  uint64_t raw; // used only to zeroing the register when initializing
};
static void initIOAPIC();

// enable an HW IRQ and connects it to an IDT entry. the apic_id is the id of
// the apic to which send the interrupt and 0 is the default core
static void enableIrqIOAPIC(uint8_t irq, uint8_t vector, uint8_t apic_id = 0);

// enable or disable a specific IRQ. if masked is disabled
static void setMaskIOAPIC(uint8_t irq, bool masked);

} // namespace miosix
