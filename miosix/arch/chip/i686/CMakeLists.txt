##
## CMakeLists.txt for i686
##

# CPU microarchitecture
set(MIOSIX_CPU_INC ${CMAKE_CURRENT_SOURCE_DIR}/arch/cpu/i686)

# Set the path to the multilib directory for this architecture (used by clang)
set(MIOSIX_MULTILIB_PATH "")

# Select appropriate compiler flags for both ASM/C/C++/linker
set(MIOSIX_CPU_FLAGS "-m32 -march=i686")
set(MIOSIX_CHIP_C_FLAGS "-D_CHIP_I686 -ffreestanding -fno-pie")
set(MIOSIX_CHIP_CXX_FLAGS "-D_CHIP_I686 -ffreestanding -fno-pie -fno-exceptions -fno-rtti")

# Select architecture specific files
set(MIOSIX_CHIP_SRC
    ${MIOSIX_CPU_INC}/interfaces-impl/gdt_asm.s
    ${MIOSIX_CPU_INC}/interfaces-impl/idt_asm.s
    ${MIOSIX_CPU_INC}/interfaces-impl/cache.cpp
    ${MIOSIX_CPU_INC}/interfaces-impl/cpu.cpp
    ${MIOSIX_CPU_INC}/interfaces-impl/gdt.cpp
    ${MIOSIX_CPU_INC}/interfaces-impl/idt.cpp
    ${MIOSIX_CPU_INC}/interfaces-impl/interrupts.cpp
    ${MIOSIX_CPU_INC}/interfaces-impl/ioapic.cpp
    ${MIOSIX_CPU_INC}/interfaces-impl/lapic.cpp
    ${MIOSIX_CPU_INC}/interfaces-impl/mmu.cpp
    ${MIOSIX_CPU_INC}/interfaces-impl/userspace.cpp
)
