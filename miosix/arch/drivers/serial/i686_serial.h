/***************************************************************************
 *   Copyright (C) 2024,2025 by Daniele Cattaneo                           *
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

#include "filesystem/console/console_device.h"
#include "interfaces/arch_registers.h"
#include "kernel/queue.h"
#include "kernel/sync.h"

namespace miosix {
class I686Serial : public Device {
public:
  /**
   * \param id 1 for COM1 (0x3F8, IRQ4), 2 for COM2 (0x2F8, IRQ3),
   *           3 for COM3 (0x3E8, IRQ4), 4 for COM4 (0x2E8, IRQ3)
   * \param baudrate serial port baudrate (max 115200)
   */
  I686Serial(int id, int baudrate);

  /**
   * Read a block of data
   * \param buffer buffer where read data will be stored
   * \param size buffer size
   * \param where where to read from
   * \return number of bytes read or a negative number on failure. Note that
   * it is normal for this function to return less character than the amount
   * asked
   */
  ssize_t readBlock(void *buffer, size_t size, off_t where);

  /**
   * Write a block of data
   * \param buffer buffer where take data to write
   * \param size buffer size
   * \param where where to write to
   * \return number of bytes written or a negative number on failure
   */
  ssize_t writeBlock(const void *buffer, size_t size, off_t where);

  /**
   * Write a string.
   * An extension to the Device interface that adds a new member function,
   * which is used by the kernel on console devices to write debug information
   * before the kernel is started or in case of serious errors, right before
   * rebooting.
   * \param str the string to write. The string must be NUL terminated.
   */
  void IRQwrite(const char *str);

  /**
   * Performs device-specific operations
   * \param cmd specifies the operation to perform
   * \param arg optional argument that some operation require
   * \return the exact return value depends on CMD, -1 is returned on error
   */
  int ioctl(int cmd, void *arg);

  /**
   * Destructor
   */
  ~I686Serial();

private:
  /**
   * \internal the serial port interrupts call this member function.
   * Never call this from user code.
   */
  void IRQhandleInterrupt() noexcept;

  void waitTxEmpty();

  const unsigned short base; // Base address of the port
  static const unsigned int rxQueueMin = 1;
  const unsigned int irqn; ///< Interrupt number
  const unsigned char portId;
  KernelMutex txMutex; ///< Mutex locked during transmission
  KernelMutex rxMutex; ///< Mutex locked during reception
  /// Semaphore flagged when the hardware TX FIFO is ready to receive bytes
  Semaphore txLowWaterFlag;
  /// Software queue used for buffering bytes from the hardware RX FIFO
  DynUnsyncQueue<char> rxQueue;

  Thread *rxWaiting; ///< Thread waiting for rx, or 0
  bool idle = false;
};

} // namespace miosix
