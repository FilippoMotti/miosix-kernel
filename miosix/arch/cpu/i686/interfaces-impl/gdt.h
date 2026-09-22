#include <stdint.h>

struct GDTEntryStruct {
  uint16_t limit;
  uint16_t base_low;
  uint8_t base_middle;
  uint8_t access;
  uint8_t flags;
  uint8_t base_high;
} __attribute__((
    packed)); // used to tell the compiler to use as little memory as possible

struct GDTPointerStruct {
  uint16_t limit;
  uint32_t base;
} __attribute__((packed));

void initGDT();
void setGDTGate(
    uint32_t num, uint32_t base, uint32_t limit, uint8_t access,
    uint8_t gran); // altough limit is 20 bits, we don't have a uint20_t
