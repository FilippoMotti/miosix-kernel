/***************************************************************************
 *   Copyright (C) 2024 by Daniele Cattaneo                                *
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

#include "arch/cpu/i686/interfaces-impl/hpet.h"
#include "arch/cpu/i686/interfaces-impl/interrupts.h"
#include "arch/cpu/i686/interfaces-impl/ioapic.h"

#include "arch/cpu/i686/interfaces-impl/lapic.h"

#include "interfaces/arch_registers.h"
#include "interfaces/delays.h"
#include "interfaces/interrupts.h"
#include "interfaces_private/cpu.h"
#include "interfaces_private/os_timer.h"
#include "kernel/lock.h"
#include "miosix_settings.h"
#include <cstdint>

namespace miosix {

#define HPET_IRQ 49
#define LAPIC_TIMER_IRQ 32
static TimeConversion tc;
static long long lastAlarmTicks = 0x7FFFFFFFFFFFFFFFLL;
static long long irqNs = 0x7FFFFFFFFFFFFFFFLL;
static unsigned int periodFs;
static uint32_t apicID;
static unsigned int hpetFreq;

// arm the hpet comparator, but if the tick is already past it will send
// directly the ipi as comparator will never do
static void setComparator(long long ticks) {
  if (!HPETArmComparator(ticks, 0))
    APICSendIPI(apicID, HPET_IRQ);
}
/**
 * \internal
 * Get raw tick count from the timer.
 * \returns the current tick count.
 */
static inline long long IRQgetTicks() noexcept { return HPETRead(); }

long long getTime() noexcept {
  FastGlobalIrqLock dLock;
  return tc.tick2ns(IRQgetTicks());
}

long long IRQgetTime() noexcept { return tc.tick2ns(IRQgetTicks()); }

/**
 * \internal
 * Set the current system time.
 * It is used by the kernel, and should not be used by end users.
 * Used to adjust the time for example if the system clock was stopped due to
 * entering deep sleep.
 * Can be called with interrupts disabled or within an interrupt.
 * \param ns value to set the hardware timer to. Note that the timer can
 * only be set to a higher value, never to a lower one, as the OS timer
 * needs to be monotonic.
 * If an interrupt has been set with IRQsetNextInterrupt, it needs to be
 * moved accordingly or fired immediately if the timer advance causes it
 * to be in the past.
 */
void IRQosTimerSetTime(long long ns) noexcept {

  long long newTicks = tc.ns2tick(ns);

  HPETStop();
  long long oldTicks = HPETRead();
  bool late = false;
  if (newTicks > oldTicks) {
    HPETSetTime(newTicks);
    if (lastAlarmTicks != 0x7FFFFFFFFFFFFFFFLL && newTicks >= lastAlarmTicks) {
      APICSendIPI(apicID, HPET_IRQ);
    }
  }
  HPETStart();
}

// handler for the hpet
// it is made similar as the os_timer class handler
void HPETIrqHandler(void *) {
  FastGlobalLockFromIrq lock;
  long long lastTick = lastAlarmTicks;
  if (lastTick == 0x7FFFFFFFFFFFFFFFLL)
    return;
  long long nowTicks = IRQgetTicks();
  if (lastTick <= nowTicks && tc.tick2ns(nowTicks) >= irqNs) {
    long long t = irqNs;
    irqNs = lastAlarmTicks = 0x7FFFFFFFFFFFFFFFLL;
    IRQwakeThreads(t);
  } else {
    if (lastTick <= nowTicks)
      lastAlarmTicks = nowTicks + 1;
    setComparator(lastAlarmTicks);
  }
}

// handler for when the time slice ends and will need to invoke the scheduler
static void IRQTimerInterruptHandler(void *) { IRQinvokeScheduler(); }
/**
 * \internal
 * Initialize and start the os timer.
 * It is used by the kernel, and should not be used by end users.
 */
void IRQosTimerInit() {
  GlobalIrqLock lock; // does nothing, but is needed by IRQregisterIrq
  setupHPET();
  periodFs = HPETgetPeriod();
  apicID = APICGetID();

  int pin = HPETChoosePin(0);
  /*if (pin < 0) chiamare errore */

  hpetFreq = 1000000000000000ULL / periodFs;
  tc = TimeConversion(hpetFreq);
  // IRQs number 17 aka vector 49
  IRQregisterIrq(lock, HPET_IRQ - 32, HPETIrqHandler, nullptr);
  // Setting the link between hpet and idt through ioapic
  enableIrqIOAPIC(pin, HPET_IRQ, apicID);
  setMaskIOAPIC(pin, false);
  IRQregisterIrq(lock, LAPIC_TIMER_IRQ - 32, IRQTimerInterruptHandler, nullptr);

  HPETEnableComparator(0, pin);
  // LAPIC timer should be on IRQ0 aka vector 32
  initLAPICTimerHPET(LAPIC_TIMER_IRQ);
}

#ifdef WITH_SMP
/**
 * \internal
 * Initialize the OS timer for a given core during SMP setup.
 * This function is used by the kernel, and should not be used by end users,
 * and is called by SMP setup code. On non-SMP platforms it is not called.
 */
void IRQosTimerInitSMP() {
#ifdef OS_TIMER_MODEL_UNIFIED
  if (getCurrentCoreId() == 0) {
    IRQregisterIrqOnCurrentCore(TIMER_IRQ_0_IRQn, IRQtimerInterruptHandler<0>,
                                nullptr);
  } else {
    IRQregisterIrqOnCurrentCore(TIMER_IRQ_1_IRQn, IRQtimerInterruptHandler<1>,
                                nullptr);
  }
#else  // OS_TIMER_MODEL_UNIFIED
  IRQinitCoreLocalPreemptionTimer();
  if (getCurrentCoreId() == WAKEUP_HANDLING_CORE)
    IRQregisterIrqOnCurrentCore(TIMER_IRQ_1_IRQn,
                                IRQtimerInterruptHandler<WAKEUP_HANDLING_CORE>,
                                nullptr);
#endif // OS_TIMER_MODEL_UNIFIED
}

#ifdef OS_TIMER_MODEL_UNIFIED
void IRQosTimerSetPreemption(unsigned int ns) noexcept {
  int coreId = getCurrentCoreId();
  // Writing to the ALARM register also enables the timer
  unsigned int ticks = ns / 21; // Imprecise conversion is acceptable here
  unsigned int alarm = timer_hw->timerawl + ticks;
  timer_hw->alarm[coreId] = alarm;
  if (timer_hw->timerawl >= alarm) {
    if (coreId == 0)
      NVIC_SetPendingIRQ(TIMER_IRQ_0_IRQn);
    else
      NVIC_SetPendingIRQ(TIMER_IRQ_1_IRQn);
  }
}
#endif // OS_TIMER_MODEL_UNIFIED
#endif // WITH_SMP

/**
 * \internal
 * Set the next interrupt on the current core.
 * It is used by the kernel, and should not be used by end users.
 * Can be called with interrupts disabled or within an interrupt.
 * The hardware timer handles only one outstading interrupt request at a
 * time, so a new call before the interrupt expires cancels the previous one.
 * \param ns the absolute time when the interrupt will be fired, in
 * nanoseconds. When the interrupt fires, it shall call the
 * \code
 * void IRQwakeThreads(long long currentTime);
 * \endcode
 * function defined in thread.cpp
 */
void IRQosTimerSetInterrupt(long long ns) noexcept {
  irqNs = ns;
  lastAlarmTicks = tc.ns2tick(ns);
  setComparator(lastAlarmTicks);
}

/**
 * \return if the last timer interrupt time that was scheduled with
 * IRQosTimerSetInterrupt() is still pending, return the abolute time in
 * nanoseconds of the pending interrupt. If the interrupt time already passed
 * no further interrupt has been set, return numeric_limits<long long>::max()
 */
long long IRQosTimerGetInterrupt() noexcept { return irqNs; }

/**
 * \internal
 * It is used by the kernel, and should not be used by end users.
 * \return the timer frequency in Hz.
 * If a prescaler is used, it should be taken into account, the returned
 * value should be equal to the frequency at which the timer increments in
 * an observable way through IRQgetCurrentTime()
 */
unsigned int osTimerGetFrequency() { return hpetFreq; }

void IRQosTimerSetPreemption(unsigned int ns) noexcept {
  APICTimerSetPreemption(ns);
}

} // namespace miosix
