#include "hpet.h"
#include <cstdint>

namespace miosix {
static constexpr uintptr_t HPET_BASE = 0xFED00000;
static volatile HPETEntryStruct *hpet;
static unsigned int periodFs;

// since we can read 32 bit at time, as suggested in the intel hpet
// documentation, we read first the high 32 bits, then the low 32 bits, then the
// high 32 bits again. If the high 32 bits have not changed between the two
// reads, then a rollover has not happened and the low 32 bits are valid. If the
// high 32 bits have changed between reads, then the multiple reads are repeated
// until a valid read is performed.
unsigned long long HPETRead() {
  uint32_t hi, lo, hi2;
  do {
    hi = hpet->counter.hi;
    lo = hpet->counter.lo;
    hi2 = hpet->counter.hi;
  } while (hi != hi2);
  return (static_cast<unsigned long long>(hi) << 32) | lo;
}

unsigned int HPETgetPeriod() { return periodFs; }

bool HPETArmComparator(unsigned long long target, int number) {
  hpet->timer[number].comparator.hi = static_cast<uint32_t>(target >> 32);
  hpet->timer[number].comparator.lo = static_cast<uint32_t>(target);
  if (target <= HPETRead())
    return false;
  return true;
}

int HPETChoosePin(int number) {
  // first determine allowed IO APIC routing
  uint32_t allowed_routes = hpet->timer[number].config.hi;
  for (int used_route = 3; used_route < 16; used_route++)
    if ((allowed_routes & 1) == 0) {
      allowed_routes >>= 1;
      return used_route;
    }
  return -1; // no usable pin
}

void HPETStop() { hpet->config.lo = 0; }

void HPETStart() { hpet->config.lo = 1; }

void HPETSetTimer(long long time) {
  hpet->counter.hi = time >> 32;
  hpet->counter.lo = time;
}

void HPETEnableComparator(int number, int pin) {
  // set route and enable interrupts
  hpet->timer[number].config.lo = (static_cast<uint32_t>(pin) << 9) | (1 << 2);
}
void setupHPET() {

  hpet = (HPETEntryStruct *)HPET_BASE;
  periodFs = hpet->capabilities.hi;

  hpet->config.lo = 0; // Stop and zeroed the counter
  hpet->counter.lo = 0;
  hpet->counter.hi = 0;

  hpet->config.lo = 1; // start the counter
}
} // namespace miosix
