#pragma once

#include "filesystem/console/console_device.h"
#include "interfaces/arch_registers.h"
#include "kernel/queue.h"
#include "kernel/sync.h"

namespace miosix {

/**
 * Output-only console on the VGA text mode buffer (80x25 at 0xB8000).
 * Meant for PCs without a usable UART. Input is not supported (no keyboard
 * driver yet): readBlock() blocks forever.
 * Classes of this type are reference counted, must be allocated on the heap
 * and managed through intrusive_ref_ptr<Device>
 */
class VGAConsole : public Device {
public:
  VGAConsole();

  ssize_t readBlock(void *buffer, size_t size, off_t where);

  ssize_t writeBlock(const void *buffer, size_t size, off_t where);

  /**
   * Write a string without taking any lock. Used before the kernel is
   * started or for fatal errors.
   */
  void IRQwrite(const char *str);

  int ioctl(int cmd, void *arg);

private:
  void print(char c);
  void scrollUp();
  void updateCursor();
  void Reset();
  void newLine();

  uint16_t defaultColor;
  uint16_t currenctColor;

  volatile unsigned short *const vga;
  int line = 0, column = 0;
  KernelMutex mutex;
};

} // namespace miosix
