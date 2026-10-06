##
## Makefile for i686
##

## Select compiler
PREFIX := i686-miosix-elf
## CPU include directory
CPU_INC := arch/cpu/i686

## Select appropriate compiler flags for both ASM/C/C++/linker
CPU_FLAGS := -m32 -march=i686
CHIP_CFLAGS   := -D_CHIP_I686 -fno-pie $(XRAM)
CHIP_CXXFLAGS := -D_CHIP_I686 -fno-pie -fexceptions -fno-rtti $(XRAM)

## Select architecture specific files
CHIP_SRC :=                                            \
$(CPU_INC)/interfaces-impl/gdt_asm.s				   \
$(CPU_INC)/interfaces-impl/idt_asm.s				   \
$(CPU_INC)/interfaces-impl/cache.cpp				   \
$(CPU_INC)/interfaces-impl/cpu.cpp				   \
$(CPU_INC)/interfaces-impl/gdt.cpp				   \
$(CPU_INC)/interfaces-impl/idt.cpp				   \
$(CPU_INC)/interfaces-impl/interrupts.cpp			   \
$(CPU_INC)/interfaces-impl/ioapic.cpp				   \
$(CPU_INC)/interfaces-impl/lapic.cpp				   \
$(CPU_INC)/interfaces-impl/mmu.cpp				   \
$(CPU_INC)/interfaces-impl/userspace.cpp			   \
arch/drivers/os_timer/i686_os_timer.cpp				  \
arch/drivers/gpio/i686_gpio.cpp					 \
arch/drivers/sleep/i686_sleep.cpp                    \
$(CHIP_INC)/interfaces-impl/delays.cpp                   \
$(CHIP_INC)/interfaces-impl/poweroff.cpp
