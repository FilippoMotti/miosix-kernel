##
## Makefile for i686 using qemu
## Included by Makefile.inc based on the board currently selected
##
## Base directory with header files
CHIP_INC := arch/chip/i686
BOARD_SRC :=                                  \
$(BOARD_INC)/i686_boot.cpp                        \
$(BOARD_INC)/interfaces-impl/bsp.cpp		\
$(BOARD_INC)/boot.s

LINKER_SCRIPT := linker.ld

## Add a #define to allow querying board name
BOARD_CFLAGS   := -D_BOARD_I686_QEMU
BOARD_CXXFLAGS := -D_BOARD_I686_QEMU

## No need to copy on RAM since the code is already there
BOARD_CFLAGS   := -D__CODE_IN_XRAM
BOARD_CXXFLAGS := -D__CODE_IN_XRAM
