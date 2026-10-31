#include "multiboot2.h"
#include <cstdint>
#include <cstring>

namespace miosix {
extern void IRQkernelBootEntryPoint();

// we use a page pool specifically made for this, because miosix during startup
// will clean bss, and thus we need to store static values for paging in a
// different place
#define POOL __attribute__((section(".page_pool")))
#define KBASE 0x80000000
#define PAGE_FLAG_PRESENT (1 << 0)
#define PAGE_FLAG_WRITE (1 << 1)
#define NUM_PAGE_DIRS 256
#define NUM_PAGE_FRAMES (1 << 20)

POOL static uint32_t pageFrameMin;
POOL static uint32_t pageFrameMax;
POOL static uint32_t totalAlloc;

POOL uint8_t physicalMemoryBitmap[NUM_PAGE_FRAMES / 8];
POOL static uint32_t pageDirs[NUM_PAGE_DIRS][1024]
    __attribute__((aligned(4096)));
POOL static uint8_t pageDirUsed[NUM_PAGE_DIRS];

POOL static uint32_t mmio_page_tables[16][1024] __attribute__((aligned(4096)));
POOL static int mmio_table_index = 0;

static inline void flush_tlb() {
  uint32_t tmp;
  __asm__ __volatile__("movl %%cr3, %0; movl %0, %%cr3"
                       : "=r"(tmp)
                       :
                       : "memory");
}
static inline void invalidate(uint32_t vaddr) {
  asm volatile("invlpg (%0)" ::"r"(vaddr) : "memory");
}
extern "C" uint32_t initial_page_dir[];
/* Multiboot 2, unlike the original specification, uses tags
 *  to organize its contents. Each tag has an associated type,
 *  represented by a 32-bit integer. */
void init_contiguous_ram_mapping(uint32_t multiboot_addr) {

  /* The multiboot info structure begins with a 32-bit integer
   *  indicating the total size of the structure. This is then
   *  followed by a 32-bit reserved region and then by the tags. */
  uint32_t total_size = *(uint32_t *)multiboot_addr;
  uint32_t end_addr = multiboot_addr + total_size;

  struct multiboot_tag *tag = (struct multiboot_tag *)(multiboot_addr + 8);
  struct multiboot_tag_mmap *mmap_tag = nullptr;

  // Find tag mmap
  while ((uint32_t)tag < end_addr && tag->type != MULTIBOOT_TAG_TYPE_END) {
    if (tag->type == MULTIBOOT_TAG_TYPE_MMAP) {
      mmap_tag = (struct multiboot_tag_mmap *)tag;
      break;
    }

    // Align the size to 8 bytes.
    uint32_t tag_size = tag_size = tag->size;
    if ((tag_size % 8) != 0)
      tag_size += (8 - tag_size % 8);

    tag = (struct multiboot_tag *)((uint32_t)tag + tag_size);
  }

  if (!mmap_tag)
    return;

  totalAlloc = 0;
  pageFrameMin = 0xFFFFFFFF;
  pageFrameMax = 0;
  memset(physicalMemoryBitmap, 0, sizeof(physicalMemoryBitmap));
  memset(pageDirs, 0, sizeof(pageDirs));
  memset(pageDirUsed, 0, sizeof(pageDirUsed));
  memset(mmio_page_tables, 0, sizeof(mmio_page_tables));

  // find min and max page frame size
  struct multiboot_mmap_entry *entry;
  for (entry = mmap_tag->entries;
       (uint32_t)entry < (uint32_t)mmap_tag + mmap_tag->size;
       entry = (struct multiboot_mmap_entry *)((uint32_t)entry +
                                               mmap_tag->entry_size)) {

    if (entry->type == MULTIBOOT_MEMORY_AVAILABLE) { // Usable RAM ( value 1)
      uint64_t start = (entry->addr + 0xFFF) & ~0xFFFULL;
      uint64_t end = (entry->addr + entry->len) & ~0xFFFULL;

      uint32_t fmin = (uint32_t)(start / 0x1000);
      uint32_t fmax = (uint32_t)(end / 0x1000);
      if (fmin < pageFrameMin)
        pageFrameMin = fmin;
      if (fmax > pageFrameMax)
        pageFrameMax = fmax;
    }
  }
  initial_page_dir[0] = 0;
  invalidate(0);
  initial_page_dir[1023] = ((uint32_t)initial_page_dir - KBASE) |
                           PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE;

  flush_tlb();
}

void map_mmio_page(uint32_t virtual_addr, uint32_t physical_addr) {
  uint32_t pdindex = virtual_addr >> 22;
  uint32_t ptindex = (virtual_addr >> 12) & 0x3FF;

  // Here you need to check whether the PD entry is present.
  // If not, we create one
  if (!(initial_page_dir[pdindex] & 1)) {

    uint32_t *new_pt = mmio_page_tables[mmio_table_index++];

    uint32_t pt_phys = (uint32_t)new_pt - KBASE;
    initial_page_dir[pdindex] =
        pt_phys | PAGE_FLAG_WRITE | PAGE_FLAG_PRESENT; // Present | Read/Write
  }

  uint32_t pt_phys_entry = initial_page_dir[pdindex] & ~0xFFF;
  uint32_t *pt = (uint32_t *)(pt_phys_entry + KBASE);

  // Flag MMIO: Present (1) | Read/Write (2) | Cache Disable (0x10) = 0x13
  uint32_t mmio_flags = PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE | 0x10;

  pt[ptindex] = (physical_addr & ~0xFFF) | mmio_flags;

  invalidate(virtual_addr);
}

extern "C" void kernel_main(uint32_t magic, uint32_t multiboot_addr) {

  init_contiguous_ram_mapping(multiboot_addr);

  map_mmio_page(0xFEE00000, 0xFEE00000);
  map_mmio_page(0xFED00000, 0xFED00000);
  map_mmio_page(0xFEC00000, 0xFEC00000);

  miosix::IRQkernelBootEntryPoint();
}
} // namespace miosix
