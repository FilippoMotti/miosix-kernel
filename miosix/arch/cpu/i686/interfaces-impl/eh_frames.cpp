#pragma once
#include "kernel/elf_types.h"
#include <stddef.h>
namespace miosix {
// this struct has to be exact as ext_dl_phdr_info in unwind-dw2-fde-dip.c,
// since it will be casted into
struct dl_phdr_info {
  Elf32_Addr dlpi_addr;
  const char *dlpi_name;
  const Elf32_Phdr *dlpi_phdr;
  Elf32_Half dlpi_phnum;
  unsigned long long dlpi_adds;
  unsigned long long dlpi_subs;
};

const Elf32_Word PT_GNU_EH_FRAME = 0x6474e550;
extern "C" {

int dl_iterate_phdr(int (*callback)(struct dl_phdr_info *, size_t, void *),
                    void *data) {
  extern char _text[];
  extern char _etext[];
  extern char _data[];
  extern char _end[];
  extern char __GNU_EH_FRAME_HDR[];
  extern char __GNU_EH_FRAME_HDR_END[];

  static const Elf32_Phdr ph[3] = {
      {// first segment, text
       .p_type = PT_LOAD,
       .p_offset = 0,
       .p_vaddr = reinterpret_cast<Elf32_Addr>(_text),
       .p_paddr = reinterpret_cast<Elf32_Addr>(_text),
       .p_filesz = static_cast<Elf32_Word>(reinterpret_cast<Elf32_Addr>(_data) -
                                           reinterpret_cast<Elf32_Addr>(_text)),
       .p_memsz = static_cast<Elf32_Word>(reinterpret_cast<Elf32_Addr>(_data) -
                                          reinterpret_cast<Elf32_Addr>(_text)),
       .p_flags = 5, // R + X
       .p_align = 4096},
      {// second segment data and bss
       .p_type = PT_LOAD,
       .p_offset = 0,
       .p_vaddr = reinterpret_cast<Elf32_Addr>(_data),
       .p_paddr = reinterpret_cast<Elf32_Addr>(_data),
       .p_filesz = static_cast<Elf32_Word>(reinterpret_cast<Elf32_Addr>(_end) -
                                           reinterpret_cast<Elf32_Addr>(_data)),
       .p_memsz = static_cast<Elf32_Word>(reinterpret_cast<Elf32_Addr>(_end) -
                                          reinterpret_cast<Elf32_Addr>(_data)),
       .p_flags = 6, // R + W
       .p_align = 4096},
      {// third segment, eh_frame
       .p_type = PT_GNU_EH_FRAME,
       .p_offset = 0,
       .p_vaddr = reinterpret_cast<Elf32_Addr>(__GNU_EH_FRAME_HDR),
       .p_paddr = reinterpret_cast<Elf32_Addr>(__GNU_EH_FRAME_HDR),
       .p_filesz = static_cast<Elf32_Word>(
           reinterpret_cast<Elf32_Addr>(__GNU_EH_FRAME_HDR_END) -
           reinterpret_cast<Elf32_Addr>(__GNU_EH_FRAME_HDR)),
       .p_memsz = static_cast<Elf32_Word>(
           reinterpret_cast<Elf32_Addr>(__GNU_EH_FRAME_HDR_END) -
           reinterpret_cast<Elf32_Addr>(__GNU_EH_FRAME_HDR)),
       .p_flags = 4, // R
       .p_align = 4}};

  struct dl_phdr_info info;
  info.dlpi_addr = 0; // load bias, since kernel is not relocated is 0
  info.dlpi_name = "MIOSIX_KERNEL";
  info.dlpi_phdr = ph;
  info.dlpi_phnum = 3; // number of segments
  info.dlpi_adds = 1;  // number of loads
  info.dlpi_subs = 0;  // number of unloads

  return callback(&info, sizeof(info), data);
}
}

} // namespace miosix
