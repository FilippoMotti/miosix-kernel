#pragma once
#include "stdint.h"
#include <cstdint>
namespace miosix {
// we use a page pool specifically made for this, because miosix during startup
// will clean bss, and thus we need to store static values for paging in a
// different place#define POOL __attribute__((section(".page_pool")))
#define POOL __attribute__((section(".page_pool")))
#define KBASE 0x80000000
#define PAGE_FLAG_PRESENT (1 << 0)
#define PAGE_FLAG_WRITE (1 << 1)
#define PAGE_FLAG_PS (1 << 7)
#define NUM_PAGE_FRAMES (1 << 20)
#define PAGE_SIZE 0x1000
#define NUM_PAGE_DIRS 512
#define KERNEL_MALLOC 0x81400000
#define KERNEL_HEAP_LIMIT 0xFEC00000
#define REC_PAGEDIR ((uint32_t *)0xFFFFF000)
#define REC_PAGETABLE(i) ((uint32_t *)(0xFFC00000 + ((i) << 12)))
#define PAGE_FLAG_OWNER (1 << 9)
void init_contiguous_ram_mapping(uint32_t multiboot_addr);
bool map_page(uint32_t virtual_addr, uint32_t physical_addr, uint32_t flags);
int kmallocInit(uint32_t initialHeapSize);
uint32_t heapSelfTest();
} // namespace miosix
