#include "i686_vga.h"
#include "filesystem/ioctl.h"
#include "kernel/thread.h"
#include <errno.h>
#include <termios.h>

namespace miosix {
#define COLOR8_BLACK 0
#define COLOR8_LIGHT_GREY 7

#define WIDTH 80
#define HEIGHT 25

VGAConsole::VGAConsole()
    : Device(Device::TTY),
      vga(reinterpret_cast<volatile unsigned short *>(0xB8000)) {
  defaultColor = (COLOR8_LIGHT_GREY << 8) | (COLOR8_BLACK << 12);
  currenctColor = defaultColor;
  Reset();
}

ssize_t VGAConsole::readBlock(void *buffer, size_t size, off_t where) {
  // No keyboard support yet: block the caller forever, as if no key was
  // ever pressed. Returning 0 would look like end of file to stdio.
  for (;;)
    Thread::sleep(1000);
  return 0;
}

ssize_t VGAConsole::writeBlock(const void *buffer, size_t size, off_t where) {
  Lock<KernelMutex> l(mutex);
  const char *buf = reinterpret_cast<const char *>(buffer);
  for (size_t i = 0; i < size; i++)
    print(buf[i]);
  return size;
}

void VGAConsole::IRQwrite(const char *str) {
  while (*str)
    print(*str++);
}

int VGAConsole::ioctl(int cmd, void *arg) {
  if (reinterpret_cast<unsigned>(arg) & 0b11)
    return -EFAULT; // Unaligned
  termios *t = reinterpret_cast<termios *>(arg);
  switch (cmd) {
  case IOCTL_SYNC:
    return 0;
  case IOCTL_TCGETATTR:
    t->c_iflag = IGNBRK | IGNPAR;
    t->c_oflag = 0;
    t->c_cflag = CS8;
    t->c_lflag = 0;
    return 0;
  case IOCTL_TCSETATTR_NOW:
  case IOCTL_TCSETATTR_DRAIN:
  case IOCTL_TCSETATTR_FLUSH:
    // Changing things at runtime unsupported, so do nothing, but don't
    // return error as console_device.h implements some attribute changes
    return 0;
  default:
    return -ENOTTY;
  }
}

void VGAConsole::print(char s) {
  switch (s) {
  case '\n':
    newLine();
    break;
  case '\r':
    column = 0;
    break;
  case '\t': {
    if (column == WIDTH) {
      newLine();
    }
    uint16_t tabLen = 4 - (column % 4);
    while (tabLen != 0) {
      vga[line * WIDTH + (column++)] = ' ' | currenctColor;
    }
    break;
  }
  default:
    if (column == WIDTH) {
      newLine();
    }
    vga[line * WIDTH + (column++)] = s | currenctColor;
    break;
  }
}

void VGAConsole::scrollUp() {
  for (uint16_t y = 0; y < HEIGHT; y++) {
    for (uint16_t x = 0; x < WIDTH; x++) {
      vga[(y - 1) * WIDTH + x] = vga[y * WIDTH + x];
    }
  }

  for (uint16_t x = 0; x < WIDTH; x++) {
    vga[(HEIGHT - 1) * WIDTH + x] = ' ' | currenctColor;
  }
}

void VGAConsole::Reset() {
  line = 0;
  column = 0;
  for (uint16_t y = 0; y < HEIGHT; y++) {
    for (uint16_t x = 0; x < WIDTH; x++) {
      vga[y * WIDTH + x] = ' ' | defaultColor;
    }
  }
}

void VGAConsole::newLine() {
  if (line < HEIGHT - 1) {
    line++;
    column = 0;
  } else {
    scrollUp();
    column = 0;
  }
}
} // namespace miosix
