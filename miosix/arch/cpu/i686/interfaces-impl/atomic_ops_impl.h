/***************************************************************************
 *   Copyright (C) 2013-2024 by Terraneo Federico                          *
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
#include <cstdint>

namespace miosix {

inline int atomicSwap(volatile int *p, int v) {
  asm volatile("xchg [%1], %0" : "+r"(v) : "r"(p) : "memory");
  return v;
}

inline void atomicAdd(volatile int *p, int incr) {
  // cc is necessary here because the add might have altered the CPU flags
  // Also lock is necessary to signal the LOCK in the bus, which prevents any
  // other processor in the system from accessing the memory at the same time
  asm volatile("lock add dword ptr [%0], %1" ::"r"(p), "r"(incr)
               : "memory", "cc");
}

inline int atomicAddExchange(volatile int *p, int incr) {
  // cc is necessary here because the add might have altered the CPU flags
  // Also lock is necessary to signal the LOCK in the bus, which prevents any
  // other processor in the system from accessing the memory at the same time
  asm volatile("lock xadd [%1], %0" : "+r"(incr) : "r"(p) : "memory", "cc");
  return incr;
}

inline int atomicCompareAndSwap(volatile int *p, int prev, int next) {
  // cc is necessary here because the compare might have altered the CPU flags
  // Also lock is necessary to signal the LOCK in the bus, which prevents any
  // other processor in the system from accessing the memory at the same time
  int result;
  asm volatile("lock cmpxchg [%1], %2"
               : "=a"(result)
               : "r"(p), "r"(next), "a"(prev)
               : "memory", "cc");
  return result;
}

inline void *atomicFetchAndIncrement(void *const volatile *p, int offset,
                                     int incr) {
  // we achieave atomic fetch and increment by emulating LL/SC
  void *result;
  volatile uint32_t *rcp;
  int rc;
  for (;;) {
    result = *p;
    if (result == 0)
      return 0;
    rcp = reinterpret_cast<uint32_t *>(result) + offset;
    rc = *rcp;
    asm volatile("" ::: "memory");
    // If something has modified the original pointer then we have to redo from
    // start
    if (result != *p)
      continue;
    if (atomicCompareAndSwap(reinterpret_cast<volatile int *>(rcp), rc,
                             rc + incr) == rc) {
      break;
    }
  }
  return result;
}

} // namespace miosix
