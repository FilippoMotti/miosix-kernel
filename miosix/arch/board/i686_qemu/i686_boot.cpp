#include "arch/cpu/i686/interfaces-impl/mmu.h"
#include "stdint.h"

namespace miosix {
extern void IRQkernelBootEntryPoint();

extern "C" void kernel_main(uint32_t magic, uint32_t multiboot_addr) {

  setMultibootInfo(multiboot_addr);
  miosix::IRQkernelBootEntryPoint();
}
} // namespace miosix
