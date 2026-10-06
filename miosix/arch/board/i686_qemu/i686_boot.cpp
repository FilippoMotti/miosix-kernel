#include <cstdint>

uint32_t saved_multiboot_info = 0;

namespace miosix {
extern void IRQkernelBootEntryPoint();
}

extern "C" void kernel_main(uint32_t magic, uint32_t info_addr) {
  if (magic == 0x2BADB002) {
    saved_multiboot_info = info_addr;
  }

  miosix::IRQkernelBootEntryPoint();
}
