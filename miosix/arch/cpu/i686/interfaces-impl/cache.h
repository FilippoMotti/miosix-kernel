/***************************************************************************
 *   Copyright (C) 2018-2026 by Terraneo Federico                          *
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

/*
 * On x86 cache coherency is enforced by the hardware, so we keep these function
 * for compatibility reason
 */

#include "interfaces/arch_registers.h"

namespace miosix {

static const unsigned int cacheLine = 64; // x86 use a line cache of 64 bytes

inline void markBufferBeforeDmaWrite(const void *buffer, int size) {
  // Fence just to be sure the compiler doesn't reorder things
  asm volatile("" ::: "memory");
}

inline void markBufferAfterDmaRead(void *buffer, int size) {
  // Fence just to be sure the compiler doesn't reorder things
  asm volatile("" ::: "memory");
}

inline void IRQenableCache() {
  // Cache is enabled and disabled by setting the CD bit of CR0 on boot
}

} // namespace miosix
