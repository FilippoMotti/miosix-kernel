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

#include "interfaces_private/cpu.h"
#include "interfaces/arch_registers.h"
#include "interfaces_private/userspace.h"

namespace miosix {

void initKernelThreadCtxsave(unsigned int *ctxsave,
                             void (*pc)(void *(*)(void *), void *),
                             unsigned int *sp, unsigned int *spLimit,
                             void *(*arg0)(void *), void *arg1) noexcept {
  unsigned int *stackPtr = sp;
  // Stack is full descending, so decrement first

  // On x86 argument are pushed on the stack before calling the context switch,
  // and they are pushed in reverse
  stackPtr--;
  *stackPtr = reinterpret_cast<unsigned int>(arg1);
  stackPtr--;
  *stackPtr = reinterpret_cast<unsigned int>(arg0);
  // Return address of the thread
  stackPtr--;
  *stackPtr = 0xffffffff;
  // Setting for the iret function (SS and ESP are not needed since we are still
  // in ring 0)
  stackPtr--;
  *stackPtr = 0x00000200; // EFLAGS (IF bit set to 1)
  stackPtr--;
  *stackPtr = 0x00000008; // CS (GDT set for kernel code)
  stackPtr--;
  *stackPtr = reinterpret_cast<unsigned int>(pc); //--> pc/EIP

  ctxsave[0] = reinterpret_cast<unsigned int>(stackPtr); //--> esp
}

void IRQportableStartKernel() noexcept {

  /*#TODO: controllare dove finiscono questi bit settati e come andrebbero
  settati per x86
  sembrerebbe che i bit che setta sono sempre attivi non appena si carica la idt
  // Enable fault handlers
  SCB->SHCSR |= SCB_SHCSR_USGFAULTENA_Msk | SCB_SHCSR_BUSFAULTENA_Msk |
                SCB_SHCSR_MEMFAULTENA_Msk;
  // Enable traps for division by zero. Trap for unaligned memory access
  // was removed as gcc starting from 4.7.2 generates unaligned accesses by
  // default (https://www.gnu.org/software/gcc/gcc-4.7/changes.html)
  SCB->CCR |= SCB_CCR_DIV_0_TRP_Msk;
*/
  // create a temporary space to save current registers. This data is useless
  // since there's no way to stop the sheduler, but we need to save it anyway.
  unsigned int s_ctxsave[miosix::CTXSAVE_SIZE];
  ctxsave[getCurrentCoreId()] = s_ctxsave; // make global ctxsave point to it
  IRQinvokeScheduler();
  asm volatile("sti" ::
                   : "memory"); // enable hardware interrupt (memory to creare a
                                // compiler fence)
  // Never reaches here
}

} // namespace miosix
