#!/usr/bin/env bash

# Script for installing a Miosix linker script for all multilibs
# This script can also be used to update an existing install of the Miosix
# compiler to a newer miosix_process.ld

quit() {
  echo $1
  exit 1
}

if [[ -z "$PREFIX" ]]; then
  quit PREFIX must be a toolchain prefix path like '/opt/i686-miosix-elf'
fi
# Ensure PREFIX and DESTDIR are visible to the makefile
export PREFIX=$PREFIX
export DESTDIR=$DESTDIR
# For now support i686-miosix-elf only, in the future this will change
export TARGET=i686-miosix-elf

all_multilibs=$(i686-miosix-elf-gcc --print-multi-lib)
for libspec in $all_multilibs; do
  libspec_parts=(${libspec//;/ })
  export MULTILIB_PATH=${libspec_parts[0]}
  export MULTILIB_FLAGS=${libspec_parts[1]//@/ -}
  echo "Multilib path $MULTILIB_PATH (flags:$MULTILIB_FLAGS)"
  if [[ "$MULTILIB_FLAGS" == *-qkernelspace* ]]; then
    echo 'Kernel multilib, skipped.'
  else
    $SUDO install -m 644 miosix_process.ld \
      $DESTDIR$PREFIX/$TARGET/lib/$MULTILIB_PATH || quit "Install failed!"
  fi
done
make clean
