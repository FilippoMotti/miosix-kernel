#pragma once
#include "stdint.h"

namespace miosix {
struct addressStructure {
  uint8_t address_space_id; // 0 - system memory, 1 - system I/O
  uint8_t register_bit_width;
  uint8_t register_bit_offset;
  uint8_t reserved;
  uint64_t address;
} __attribute__((packed));

struct descriptionTableHeader {
  char signature[4]; // 'HPET' in case of HPET table
  uint32_t length;
  uint8_t revision;
  uint8_t checksum;
  char oemid[6];
  uint64_t oem_tableid;
  uint32_t oem_revision;
  uint32_t creator_id;
  uint32_t creator_revision;
} __attribute__((packed));

struct hpet : public descriptionTableHeader {
  uint8_t hardware_rev_id;
  uint8_t comparator_count : 5;
  uint8_t counter_size : 1;
  uint8_t reserved : 1;
  uint8_t legacy_replacement : 1;
  uint16_t pci_vendor_id;
  addressStructure address;
  uint8_t hpet_number;
  uint16_t minimum_tick;
  uint8_t page_protection;
} __attribute__((packed));

// This division because we have 32 bit bus
struct hpet_reg64 {
  volatile uint32_t lo;
  volatile uint32_t hi;
};

struct hpet_timer {  // 0x20 byte per comparatore
  hpet_reg64 config; // bit 1 level, 2 INT_ENB, 3 periodic, 9-13 pin IOAPIC
                     // hi = pin consentiti (routing capability)
  hpet_reg64 comparator;
  hpet_reg64 fsb_route;
  hpet_reg64 reserved;
};

struct HPETEntryStruct {
  hpet_reg64
      capabilities;      // 0x000 General Capabilities and ID Register (R only)
  hpet_reg64 reserved0;  // 0x008
  hpet_reg64 config;     // 0x010 General Configuration Register (RW)
  hpet_reg64 reserved1;  // 0x018
  hpet_reg64 int_status; // 0x020 General Interrupt Status Register (RW clear)
  hpet_reg64 reserved2[25]; // 0x028 - 0x0EF
  hpet_reg64 counter;       // 0x0F0 Main Counter Value Register (RW)
  hpet_reg64 reserved3;     // 0x0F8
  hpet_timer timer[32];     // 0x100, 32 timers (RW) divided in
  // 0->7 Timer Configuration and Capability Register
  // 8->F Timer Comparator Value Register
  // 10->17 Timer FSB Interrupt Route Register
  // 18->1F padding
};

void setupHPET();
unsigned long long HPETRead();
unsigned int HPETgetPeriod();
int HPETChoosePin(int number);
void HPETEnableComparator(int number, int pin);
bool HPETArmComparator(unsigned long long target, int number);
void HPETStart();
void HPETStop();
void HPETSetTime(long long time);
} // namespace miosix
