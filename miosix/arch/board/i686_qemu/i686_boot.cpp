#include "paging.h"
#include "stdint.h"

namespace miosix {
extern void IRQkernelBootEntryPoint();

extern "C" void kernel_main(uint32_t magic, uint32_t multiboot_addr) {

  init_contiguous_ram_mapping(multiboot_addr);

  kmallocInit(KERNEL_HEAP_LIMIT - KERNEL_MALLOC);

  map_page(0xFEE00000, 0xFEE00000, PAGE_FLAG_WRITE | 0x10);
  map_page(0xFED00000, 0xFED00000, PAGE_FLAG_WRITE | 0x10);
  map_page(0xFEC00000, 0xFEC00000, PAGE_FLAG_WRITE | 0x10);
  miosix::IRQkernelBootEntryPoint();
}
} // namespace miosix
