#include "i686_serial.h"
#include "arch/cpu/i686/interfaces-impl/ioapic.h"
#include "arch/cpu/i686/interfaces-impl/lapic.h"
#include "filesystem/ioctl.h"
#include "interfaces/interrupts.h"
#include "kernel/logging.h"
#include "kernel/sync.h"
#include <cstring>
#include <errno.h>
#include <termios.h>

using namespace miosix;

namespace {

#define COM1 0x3F8
#define COM2 0x2F8
#define COM3 0x3E8
#define COM4 0x2E8

// 16550 registers, offsets from the base port
#define RBR 0 // receive buffer (read), DLL with DLAB=1
#define THR 0 // transmit holding (write)
#define IER 1 // interrupt enable, DLM with DLAB=1
#define IIR 2 // interrupt id (read)
#define FCR 2 // fifo control (write)
#define LCR 3 // line control
#define MCR 4 // modem control
#define LSR 5 // line status
#define MSR 6 // modem status

struct PortInfo {
  unsigned short base;
  unsigned char irqn;
};
constexpr PortInfo ports[] = {{COM1, 4}, {COM2, 3}, {COM3, 4}, {COM4, 3}};
constexpr int maxPorts = sizeof(ports) / sizeof(ports[0]);

inline void outb(unsigned short p, unsigned char v) {
  asm volatile("outb %0,%1" ::"a"(v), "Nd"(p));
}

inline unsigned char inb(unsigned short p) {
  unsigned char v;
  asm volatile("inb %1,%0" : "=a"(v) : "Nd"(p));
  return v;
}

} // anonymous namespace

//
// class I686Serial
//

I686Serial::I686Serial(int id, int baudrate)
    : Device(Device::TTY),
      base(ports[(id < 1 || id > maxPorts) ? 0 : id - 1].base),
      irqn(ports[(id < 1 || id > maxPorts) ? 0 : id - 1].irqn),
      rxQueue(rxQueueMin + baudrate / 500) {
  if (id < 1 || id > maxPorts) {
    errorHandler(Error::UNEXPECTED);
  }
  if (baudrate <= 0 || baudrate > 115200) {
    errorHandler(Error::UNEXPECTED);
  }
  unsigned short divisor = 115200 / baudrate;

  // Setting the divisor to the controller
  GlobalIrqLock dLock;
  outb(base + IER, 0x00);         // Disable all interrupts
  outb(base + LCR, 0x80);         // Enable DLAB (set baud rate divisor)
  outb(base + 0, divisor & 0xff); // Divisor Last significat bit
  outb(base + 1, divisor >> 8);   // Divisor most significabt bit
  outb(base + LCR, 0x03);         // 8 bits, no parity, one stop bit
  outb(base + FCR, 0xC7); // Enable FIFO, clear them, with 14-byte threshold
  outb(base + MCR, 0x0B); // IRQs enabled, RTS/DSR set
  outb(base + MCR, 0x1E); // Set in loopback mode, test the serial chip
  outb(base + THR, 0xAE); // Test serial chip (send byte 0xAE and check if
                          // serial returns same byte)
  // Check if serial is faulty (i.e: not same byte as sent)
  if (inb(base + RBR) != 0xAE) {
    errorHandler(Error::UNEXPECTED);
  }

  // If serial is not faulty set it in normal operation mode
  // (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
  outb(base + MCR, 0x0F);

  // The table slot is the ISA irqn number, vector = irq + 32. IRQregisterIrq
  // only fills IRQForwardingTable, so the IOAPIC is programmed here.
  IRQregisterIrq(dLock, irqn, &I686Serial::IRQhandleInterrupt, this);
  enableIrqIOAPIC(irqn, irqn + 32, APICGetID());
  setMaskIOAPIC(irqn, false);

  outb(base + IER, 0x01); // interrupt on received data (the
                          // fifo timeout interrupt comes with it)
}

ssize_t I686Serial::readBlock(void *buffer, size_t size, off_t where) {

  Lock<KernelMutex> l(rxMutex);
  char *buf = reinterpret_cast<char *>(buffer);
  size_t result = 0;
  FastGlobalIrqLock dLock;
  DeepSleepLock dpLock;
  for (;;) {
    // Try to get data from the queue
    for (; result < size; result++) {
      if (rxQueue.tryGet(buf[result]) == false)
        break;
      // This is here just not to keep IRQ disabled for the whole loop
      FastGlobalIrqUnlock eLock(dLock);
    }
    if (idle && result > 0)
      break;
    if (result == size)
      break;
    // Wait for data in the queue
    rxWaiting = Thread::IRQgetCurrentThread();
    do
      Thread::IRQglobalIrqUnlockAndWait(dLock);
    while (rxWaiting);
  }
  return result;
}

ssize_t I686Serial::writeBlock(const void *buffer, size_t size, off_t where) {
  Lock<KernelMutex> l(txMutex);
  const char *buf = reinterpret_cast<const char *>(buffer);
  for (size_t i = 0; i < size; i++) {
    while ((inb(base + LSR) & 0x20) == 0)
      ;
    outb(base + THR, *buf++);
  }
  return size;
}

void I686Serial::IRQwrite(const char *str) {
  while (*str) {
    while ((inb(base + LSR) & 0x20) == 0)
      ;
    outb(base + THR, *str++);
  }
  waitTxEmpty();
}

void I686Serial::waitTxEmpty() {
  while ((inb(base + LSR) & 1 << 6) == 0)
    ;
}

int I686Serial::ioctl(int cmd, void *arg) {
  if (reinterpret_cast<unsigned>(arg) & 0b11)
    return -EFAULT; // Unaligned
  termios *t = reinterpret_cast<termios *>(arg);
  switch (cmd) {
  case IOCTL_SYNC:
    waitTxEmpty();
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

void I686Serial::IRQhandleInterrupt() noexcept {
  for (;;) {
    // checking if there are new interrupts
    // as the bit 0 says if there are interupt pending if set to 0
    unsigned char iir = inb(base + IIR);
    if (iir & 1)
      break; // No more interrupts pending
    // check if timeout interrupt pending before a read can clear it
    bool timeout = iir >> 3 & 1;
    // checking bit 1-2
    // this is following priority order from highest to lower
    switch ((iir >> 1) & 0x3) {
    case 3:            // line status
      inb(base + LSR); // reading clears the condition
      break;
    case 2: // received data available
      unsigned char lsr;
      while ((lsr = inb(base + LSR)) & 1) {
        idle = false;
        // always read data, since this clears interrupt flags
        char c = inb(base + RBR);
        // if no framing error put data in buffer
        if ((lsr >> 3 & 1) == 0)
          if (rxQueue.tryPut(c) == false) /*fifo overflow*/
            ;
      }
      break;
    case 1:  // Transmitter Holding Register Empty
      break; // reading iir clears it
    case 0:  // modem status
      inb(base + MSR);
      break;
    }

    if (timeout) {
      idle = true;
    }
  }
  if ((idle || rxQueue.size() >= rxQueueMin) && rxWaiting) {
    rxWaiting->IRQwakeup();
    rxWaiting = nullptr;
  }
}

I686Serial::~I686Serial() {
  waitTxEmpty();
  GlobalIrqLock dLock;
  outb(base + IER, 0x00);
  setMaskIOAPIC(irqn, true);
  IRQunregisterIrq(dLock, irqn, &I686Serial::IRQhandleInterrupt, this);
}
