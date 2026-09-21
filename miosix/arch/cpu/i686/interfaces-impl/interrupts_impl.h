/***************************************************************************
 *   Copyright (C) 2010-2024 by Terraneo Federico                          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   As a special exception, if other files instantiate templates or use   *
 *   macros or inline functions from this file, or you compile this file   *
 *   and link it with other works to produce a work based on this file,    *
 *   this file does not by itself cause the resulting work to be covered   *
 *   by the GNU General Public License. However the source code for this   *
 *   file must still be made available in accordance with the GNU General  *
 *   Public License. This exception does not invalidate any other reasons  *
 *   why a work based on this file might be covered by the GNU General     *
 *   Public License.                                                       *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>   *
 ***************************************************************************/

#pragma once

#include "interfaces/arch_registers.h"

namespace miosix {

/**
 * \addtogroup Interfaces
 * \{
 */

// priority of an interrupt in x86 is defined by the interrupt pin (PIC) or the
// vector number of the interrupt (APIC), also 0 is lowest priority
constexpr int defaultIrqPriority = 0;

/// Minimum interrupt priority that the hardware provides
constexpr int minimumIrqPriority = 0;

inline void fastDisableIrq() noexcept {
  // Since this function is inline there's the need for a memory barrier to
  // avoid aggressive reordering
  asm volatile("cli" ::: "memory");
}

inline void fastEnableIrq() noexcept {
  // Since this function is inline there's the need for a memory barrier to
  // avoid aggressive reordering
  asm volatile("sti" ::: "memory");
}

inline bool areInterruptsEnabled() noexcept {
  unsigned int eflags;
  // Push all the flags into the stack and then pop into a variable in order to
  // read it
  asm volatile("pushf     \n\t"
               "pop %0    \n\t"
               : "=rm"(eflags)::"memory");
  // Bitwise AND with only bit 9 to check if the Interrupt Flag is active.
  return (eflags & 0x0200) != 0;
}

/**
 * \}
 */

} // namespace miosix
