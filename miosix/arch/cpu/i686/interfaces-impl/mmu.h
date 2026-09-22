#pragma once
#include "interfaces/arch_registers.h"

// MMU is always present on x86

namespace miosix {
/**
 * \internal
 * The kernel calls this function in boot.cpp to configure the MMU for
 * kernel-level W^X and cacheability (if caches are present).
 */

void IRQenableMMU();
} // namespace miosix
