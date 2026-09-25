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

#include "interrupts.h"
#include "interfaces/arch_registers.h"
#include "interfaces/interrupts.h"
#include "interfaces/poweroff.h"
#include "interfaces_private/cpu.h"
#include "interfaces_private/smp.h"
#include "kernel/boot.h"
#include "kernel/logging.h"
#include "kernel/process.h"
#include "kernel/scheduler/scheduler.h"
#include "kernel/thread.h"
#include "miosix_settings.h"
#include "util/util.h"

#include "idt.h"
#include "ioapic.h"
#include "lapic.h"

namespace miosix {

static void unexpectedInterrupt(void *);

//
// Code to build the interrupt table in FLASH and interrupt forwarding table in
// RAM
//

/**
 * \internal
 * on x86 we have 256 slots for interrupts, but the first 32 are fixed for the
 * cpu.
 */
const unsigned int numInterrupts = 224;

/**
 * \internal
 * To enable interrupt registration at run-time and interrupt arg pointer
 * passing, we use one of these structs per peripheral interrupt allocated in
 * RAM
 */
struct IrqForwardingEntry {
  // NOTE: a constexpr constructor can be used to perform static initialization
  // of the table entries, but this uses a lot of Flash memory to store the
  // initialization values. We thus opted to leave the table uninitialized
  // and add the IRQinitIrqTable function that provides space efficient
  // initialization by means of a loop
  //  constexpr IrqForwardingEntry() : handler(&unexpectedInterrupt),
  //  arg(nullptr) {}
  void (*handler)(void *);
  void *arg;
};

/// \internal Table of run-time registered interrupt handlers and args
static IrqForwardingEntry irqForwardingTable[numInterrupts];

//
void irqHandler(struct InterruptRegisters *regs) {
  if (regs->int_no >= 32 and regs->int_no < 256) {
    uint32_t id = regs->int_no - 32;

    irqForwardingTable[id].handler(irqForwardingTable[id].arg);

    // Remember to send EOI after every IRQ handled, so it can resume receiving
    // interrupts
    sendEOI();
  } else {
    IRQsystemReboot();
  }
}

//
// Implementation of the interrupts.h interface
//

void IRQinitIrqTable() noexcept {

  initIDT();

  // Turn off PIC and activate Lapic
  initLAPIC();
  // LAPIC timer should be on IRQ0 aka vector 32
  initLAPICTimer(32);

  // Initialize IO APIC
  initIOAPIC();
}

inline void IRQregisterIrqImpl(unsigned int id, void (*handler)(void *),
                               void *arg) noexcept {
  if (id >= numInterrupts ||
      irqForwardingTable[id].handler != unexpectedInterrupt)
    errorHandler(Error::INTERRUPT_REGISTRATION_ERROR);
  irqForwardingTable[id].handler = handler;
  irqForwardingTable[id].arg = arg;

  // send the IRQ to core correct core
  // also remember that irqs starts from number 32
  uint8_t vector = id + 32;
  enableIrqIOAPIC(id, vector, getIdCore());
  setMaskIOAPIC(id, false);
}

inline void IRQunregisterIrqImpl(unsigned int id, void (*handler)(void *),
                                 void *arg) noexcept {
  if (id >= numInterrupts || irqForwardingTable[id].handler != handler ||
      irqForwardingTable[id].arg != arg)
    errorHandler(Error::INTERRUPT_REGISTRATION_ERROR);
  irqForwardingTable[id].handler = unexpectedInterrupt;
#ifdef WITH_ERRLOG
  irqForwardingTable[id].arg = reinterpret_cast<void *>(id);
#endif // WITH_ERRLOG
  setMaskIOAPIC(id, true);
}

#ifdef WITH_SMP

void IRQregisterIrqOnCurrentCore(unsigned int id, void (*handler)(void *),
                                 void *arg) noexcept {
  IRQregisterIrqImpl(id, handler, arg);
}

void IRQunregisterIrqOnCurrentCore(unsigned int id, void (*handler)(void *),
                                   void *arg) noexcept {
  IRQunregisterIrqImpl(id, handler, arg);
}

struct IrqRegistrationContext {
  unsigned int id;
  void (*handler)(void *);
  void *arg;
};

static void IRQregisterIrqOnCoreHandler(void *ctxt) {
  IrqRegistrationContext *irqCtxt =
      reinterpret_cast<IrqRegistrationContext *>(ctxt);
  IRQregisterIrqImpl(irqCtxt->id, irqCtxt->handler, irqCtxt->arg);
}

void IRQregisterIrqOnCore(GlobalIrqLock &lock, unsigned char coreId,
                          unsigned int id, void (*handler)(void *),
                          void *arg) noexcept {
  if (coreId == getCurrentCoreId())
    IRQregisterIrqImpl(id, handler, arg);
  else {
    IrqRegistrationContext irqCtxt = {id, handler, arg};
    IRQcallOnCore(lock, coreId, &IRQregisterIrqOnCoreHandler, &irqCtxt);
  }
}

static void IRQunregisterIrqOnCoreHandler(void *ctxt) {
  IrqRegistrationContext *irqCtxt =
      reinterpret_cast<IrqRegistrationContext *>(ctxt);
  IRQunregisterIrqImpl(irqCtxt->id, irqCtxt->handler, irqCtxt->arg);
}

void IRQunregisterIrqOnCore(GlobalIrqLock &lock, unsigned char coreId,
                            unsigned int id, void (*handler)(void *),
                            void *arg) noexcept {
  if (coreId == getCurrentCoreId())
    IRQunregisterIrqImpl(id, handler, arg);
  else {
    IrqRegistrationContext irqCtxt = {id, handler, arg};
    IRQcallOnCore(lock, coreId, &IRQunregisterIrqOnCoreHandler, &irqCtxt);
  }
}

#else // WITH_SMP

void IRQregisterIrq(GlobalIrqLock &lock, unsigned int id,
                    void (*handler)(void *), void *arg) noexcept {
  IRQregisterIrqImpl(id, handler, arg);
}

void IRQunregisterIrq(GlobalIrqLock &lock, unsigned int id,
                      void (*handler)(void *), void *arg) noexcept {
  IRQunregisterIrqImpl(id, handler, arg);
}

#endif // WITH_SMP

bool IRQisIrqRegistered(unsigned int id) noexcept {
  if (id >= numInterrupts)
    return false;
  return irqForwardingTable[id].handler != unexpectedInterrupt;
}

//
// Support functions to implement interrupt handlers
//

#ifdef WITH_ERRLOG
/**
 * \internal
 * Used to print an unsigned int in hexadecimal format, and to reboot the system
 * This function exists because printf/iprintf cannot be used inside an IRQ.
 * \param x number to print
 */
static void printUnsignedInt(unsigned int x) {
  char result[] = "0x........\r\n";
  formatHex(result + 2, x, 8);
  IRQerrorLog(result);
}
#endif // WITH_ERRLOG

static void unexpectedInterrupt(void *arg) {
  FastGlobalLockFromIrq lock;
#ifdef WITH_ERRLOG
  auto entryNum = reinterpret_cast<unsigned int>(arg);
  IRQerrorLog("\r\n***Caught unregistered interrupt number ");
  printUnsignedInt(entryNum);
  IRQerrorLog("\r\n");
#endif // WITH_ERRLOG
  IRQsystemReboot();
}

} // namespace miosix
