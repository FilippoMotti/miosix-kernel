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

#ifndef __FPU_PRESENT
#define __FPU_PRESENT 0 //__FPU_PRESENT undefined means no FPU
#endif
/*
 * In this architecture (i686), only ESP is saved when context switching, the
 * rest is pushed onto the stack
 * *ctxsave+0  --> esp
 */

/**
 * \internal
 * on i686 the scheduler is triggered by calling the interrupt relative to the
 * yield scheduler function in the IDT
 */

namespace miosix {

inline void IRQinvokeScheduler() noexcept {
  // 0x80 is defined as the yield scheduler in the IDT
  asm volatile("int 0x80" ::: "memory");
}

} // namespace miosix

/**
 * \}
 */
