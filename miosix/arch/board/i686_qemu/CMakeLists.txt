##
## CMakeLists.txt for board x86_qemu
##

# Directory with header files for this board
set(MIOSIX_CHIP_INC ${CMAKE_CURRENT_SOURCE_DIR}/arch/chip/i686)

# Linker script options
set(MIOSIX_LINKER_SCRIPT_LIST
    linker.ld
)
set(MIOSIX_DEFAULT_LINKER_SCRIPT linker.ld)

# Select architecture specific files
set(MIOSIX_BOARD_SRC
    ${MIOSIX_BOARD_INC}/boot.cpp
    ${MIOSIX_BOARD_INC}/boot.s
)

# Add a #define to allow querying board name and disable Flash copy
set(MIOSIX_BOARD_C_FLAGS -D_BOARD_I86_QEMU -D__CODE_IN_XRAM)
set(MIOSIX_BOARD_CXX_FLAGS -D_BOARD_I86_QEMU -D__CODE_IN_XRAM)

# Specify a custom flash command
