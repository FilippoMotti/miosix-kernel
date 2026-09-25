#include "ioapic.h"

namespace miosix {

// This is the default phisiscal base address of the io apic, there can be cases
// in which it is not, so we should read it from MADT or ACPI. (remember if
// mappped to make it uncacheable)
#define IOAPIC_BASE 0xFEC00000;

#define REG_ID 0x00  // Register index: ID
#define REG_VER 0x01 // Register index: version

static IOAPICEntryStruct *const ioapic = (IOAPICEntryStruct *)IOAPIC_BASE;

void writeIOAPIC(uint8_t reg, uint32_t val) {
  ioapic->ioapicreg = reg;
  ioapic->ioapicwin = val;
}

uint32_t readIOAPIC(uint8_t reg) {
  ioapic->ioapicreg = reg;
  return ioapic->ioapicwin;
}

void initIOAPIC() {

  uint32_t version_reg = readIOAPIC(REG_VER);
  uint32_t max_intr = (version_reg >> 16) & 0xFF;
  uint32_t regID = readIOAPIC(REG_ID >> 24);

  // Disable all IRQs
  for (uint32_t i = 0; i <= max_intr; i++) {
    setMaskIOAPIC(i, true);
  }
}

void enableIrqIOAPIC(uint8_t irq, uint8_t vector, uint8_t apic_id) {
  // formula to calculate the position of the registry in the Redirection Entry
  // Register
  uint8_t reg = 0x10 + (irq * 2);

  IORedirectionEntry entry;
  entry.raw = 0;

  entry.vector = vector;
  entry.deliveryMode = 0;    // Fixed
  entry.destinationMode = 0; // Physical
  entry.pinPolarity = 0;     // Active High
  entry.triggerMode = 0;     // Edge
  entry.mask = 0;            // Unmasked (0 = active)
  entry.destination = apic_id;

  // we first set the uppper part and then the lower part
  writeIOAPIC(reg + 1, entry.high);
  writeIOAPIC(reg, entry.low);
}

void setMaskIOAPIC(uint8_t irq, bool masked) {
  // formula to calculate the position of the registry in the Redirection Entry
  // Register
  uint8_t reg = 0x10 + (irq * 2);

  IORedirectionEntry entry;

  // extract the lower part, since the mask entry is there
  entry.low = readIOAPIC(reg);

  // we do this because a bool in memory is 8 bits and not 1
  entry.mask = masked ? 1 : 0;

  // updating only the lower part
  writeIOAPIC(reg, entry.low);
}

} // namespace miosix
