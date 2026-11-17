#pragma once
#include "paging.h"
#include "multiboot2.h"
#include "stdint.h"
#include "string.h"
namespace miosix {
POOL static uint32_t heapSize;
POOL static bool kmallocInitialized = false;

POOL uint8_t physicalMemoryBitmap[NUM_PAGE_FRAMES / 8];
POOL static uint32_t pageFrameEnd;
POOL static uint32_t pageFrameStart;

static inline void invalidate(uint32_t vaddr) {
  asm volatile("invlpg (%0)" ::"r"(vaddr) : "memory");
}

static inline void setUsed(uint32_t frame) {
  physicalMemoryBitmap[frame / 8] |= (1 << (frame % 8));
}
static inline void setFree(uint32_t frame) {
  physicalMemoryBitmap[frame / 8] &= ~(1 << (frame % 8));
}

extern "C" uint32_t initial_page_dir[];
extern "C" char _heap_end[];
extern "C" char _page_pool_start[], _page_pool_end[];

// Find the first free frame in the bitmap and return the associated address
uint32_t pmmAllocPageFrame() {

  for (uint32_t b = pageFrameStart / 8; b < (pageFrameEnd + 7) / 8; b++) {
    uint8_t byte = physicalMemoryBitmap[b];
    if (byte == 0xFF) {
      continue;
    }
    for (uint32_t i = 0; i < 8; i++) {
      bool used = byte >> i & 1;
      uint32_t addr = (b * 8 + i);

      if (!used) {
        setUsed(addr);
        pageFrameStart = addr;
        return addr * PAGE_SIZE;
      }
    }
  }
  return 0;
}
void pmmFree(uint32_t phys) {
  if (!phys)
    return;
  uint32_t frame = phys / PAGE_SIZE;
  if (frame < pageFrameEnd) {
    setFree(frame);
    if (frame < pageFrameStart)
      pageFrameStart = frame;
  }
}
// Set used a specific range of frames, this is used for case when specific
// address are needed
static void reserve(uint64_t start, uint64_t end) {
  for (uint64_t frame = start / PAGE_SIZE;
       frame < (end + PAGE_SIZE - 1) / PAGE_SIZE && frame < NUM_PAGE_FRAMES;
       frame++)
    setUsed((uint32_t)frame);
}
/* Multiboot 2, unlike the original specification, uses tags
 *  to organize its contents. Each tag has an associated type,
 *  represented by a 32-bit integer. */
void init_contiguous_ram_mapping(uint32_t multiboot_addr) {
  memset(_page_pool_start, 0, _page_pool_end - _page_pool_start);

  pageFrameStart = 0;
  pageFrameEnd = 0;

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
    uint32_t tag_size = tag->size;
    if ((tag_size % 8) != 0)
      tag_size += (8 - tag_size % 8);

    tag = (struct multiboot_tag *)((uint32_t)tag + tag_size);
  }

  if (!mmap_tag)
    return;

  // Set all frames as used
  memset(physicalMemoryBitmap, 0xFF, sizeof(physicalMemoryBitmap));

  // Find max size of a frame and free the memory frames that we actually can
  // use
  struct multiboot_mmap_entry *entry;
  for (entry = mmap_tag->entries;
       (uint32_t)entry < (uint32_t)mmap_tag + mmap_tag->size;
       entry = (struct multiboot_mmap_entry *)((uint32_t)entry +
                                               mmap_tag->entry_size)) {

    if (entry->type == MULTIBOOT_MEMORY_AVAILABLE) { // Usable RAM ( value 1)
      uint64_t start = (entry->addr + PAGE_SIZE - 1) / PAGE_SIZE;
      uint64_t end = (entry->addr + entry->len) / PAGE_SIZE;
      if (end > NUM_PAGE_FRAMES)
        end = NUM_PAGE_FRAMES;
      for (uint64_t frame = start; frame < end; frame++)
        setFree((uint32_t)frame);
      if (end > pageFrameEnd)
        pageFrameEnd = (uint32_t)end;
    }
  }
  // Reserve space for multiboot tags
  for (entry = mmap_tag->entries;
       (uint32_t)entry < (uint32_t)mmap_tag + mmap_tag->size;
       entry = (struct multiboot_mmap_entry *)((uint32_t)entry +
                                               mmap_tag->entry_size))
    if (entry->type != MULTIBOOT_MEMORY_AVAILABLE)
      reserve(entry->addr, entry->addr + entry->len);

  // Reserve space for kernel page and multiboot page
  uint32_t mbPhys =
      multiboot_addr >= KBASE ? multiboot_addr - KBASE : multiboot_addr;
  uint32_t kernelEnd = (uint32_t)_heap_end - KBASE;
  reserve(0, kernelEnd > 0x100000 ? kernelEnd : 0x100000);
  reserve(mbPhys, (uint64_t)mbPhys + total_size);

  initial_page_dir[0] = 0;
  invalidate(0);
  // Recursive mapping
  initial_page_dir[1023] = ((uint32_t)initial_page_dir - KBASE) |
                           PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE;
  invalidate(0xFFFFF000);
}
bool map_page(uint32_t virtual_addr, uint32_t physical_addr, uint32_t flags) {
  // check if we are over the recursive mapping
  if (virtual_addr >= 0xFFC00000)
    return false;

  uint32_t pdindex = virtual_addr >> 22;
  uint32_t ptindex = (virtual_addr >> 12) & 0x3FF;

  // it points to the address pointed from initial_page_dir[1023]
  uint32_t *pageDir = REC_PAGEDIR;
  uint32_t *pt = REC_PAGETABLE(pdindex);

  // check if inside a 4mb page
  if (pageDir[pdindex] & PAGE_FLAG_PS)
    return false;

  // Here you need to check whether the PD entry is present.
  // If not, we create one
  if (!(pageDir[pdindex] & PAGE_FLAG_PRESENT)) {
    uint32_t frame = pmmAllocPageFrame();
    if (!frame)
      return false;
    pageDir[pdindex] = frame | PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE;
    invalidate((uint32_t)pt);
    memset(pt, 0, PAGE_SIZE);
  }

  pt[ptindex] = (physical_addr & ~0xFFF) | PAGE_FLAG_PRESENT | flags;
  invalidate(virtual_addr);
  return true;
}
uint32_t unmap_page(uint32_t virtual_addr) {
  if (virtual_addr >= 0xFFC00000)
    return 0;

  uint32_t pdindex = virtual_addr >> 22;
  uint32_t ptindex = (virtual_addr >> 12) & 0x3FF;
  uint32_t *pageDir = REC_PAGEDIR;
  uint32_t *pt = REC_PAGETABLE(pdindex);

  if (!(pageDir[pdindex] & PAGE_FLAG_PRESENT) ||
      (pageDir[pdindex] & PAGE_FLAG_PS))
    return 0;
  if (!(pt[ptindex] & PAGE_FLAG_PRESENT))
    return 0;

  uint32_t phys = pt[ptindex] & ~0xFFF;
  pt[ptindex] = 0;
  invalidate(virtual_addr);
  return phys;
}

int changeHeapSize(int newSize) {

  if (newSize > KERNEL_HEAP_LIMIT - KERNEL_MALLOC)
    return -1;
  uint32_t oldPageTop = (heapSize + PAGE_SIZE - 1) / PAGE_SIZE;
  uint32_t newPageTop = (newSize + PAGE_SIZE - 1) / PAGE_SIZE;

  // Map all the pages to the new space (growing case)
  for (uint32_t page = oldPageTop; page < newPageTop; page++) {
    // Physical frame of the page
    uint32_t frame = pmmAllocPageFrame();

    // Map the virtual address frame to the physical frame
    if (!frame ||
        !map_page(KERNEL_MALLOC + page * PAGE_SIZE, frame, PAGE_FLAG_WRITE)) {
      // If something has gone wrong, rollback
      if (frame)
        pmmFree(frame);
      while (page > oldPageTop) {
        page--;
        pmmFree(unmap_page(KERNEL_MALLOC + page * PAGE_SIZE));
      }
      return -1;
    }
  }
  // Unmap excessive pages (shrinking case)
  for (uint32_t page = newPageTop; page < oldPageTop; page++)
    pmmFree(unmap_page(KERNEL_MALLOC + page * PAGE_SIZE));

  heapSize = newSize;
  return 0;
}
int kmallocInit(uint32_t initialHeapSize) {
  heapSize = 0;
  return changeHeapSize(initialHeapSize);
}

} // namespace miosix
