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
#include "lapic.h"

#ifndef __FPU_PRESENT
#define __FPU_PRESENT 0 //__FPU_PRESENT undefined means no FPU
#endif
/*
 * In this architecture (i686), we push all general registers on the stack
 * *ctxsave+0  --> esp
 *
 * Also when we do a pushal the registers are saved in this order
 * esp+28  --> eax
 * esp+24  --> ecx
 * esp+20  --> edx
 * esp+16  --> ebx
 * esp+12  --> esp
 * esp+8   --> ebp
 * esp+4   --> esi
 * esp+0   --> edi
 *
 * also under esp we have eip, cs and eflags that are saved by hardware
 */

#define saveContext()                                                          \
  asm volatile(                                                                \
      "pushal                  \n\t" /*eax,ecx,edx,ebx,esp,ebp,esi,edi*/       \
      "mov  ctxsave, %eax      \n\t" /*current context*/                       \
      "mov  %esp, (%eax)       \n\t" /*save sp in context*/                    \
  );
/**
 * \internal
 * on i686 the scheduler is triggered by calling the interrupt relative to the
 * yield scheduler function in the IDT
 */

#define restoreContext()                                                       \
  asm volatile("mov  ctxsave, %eax      \n\t" /*new context*/                  \
               "mov  (%eax), %esp       \n\t" /*restore esp*/                  \
               "popal                   \n\t"                                  \
               "iret                    \n\t" /*pop eip,cs,eflags*/            \
  );
namespace miosix {

inline void IRQinvokeScheduler() noexcept { setSchedulerInterruptPending(); }

} // namespace miosix

/**
 * \}
 */
