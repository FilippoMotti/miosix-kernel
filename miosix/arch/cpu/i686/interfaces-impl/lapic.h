// lapic.h
#pragma once
#include <cstdint>

namespace miosix {

// Initialize local apic and diables pic
void initLAPIC();

void sendEOI();

uint32_t getIdCore();

void initLAPICTimer(uint32_t timer_interrupt_vector);

} // namespace miosix
