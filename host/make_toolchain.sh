#!/usr/bin/env bash

set -e
set -x

CROSS_ROOT="$(pwd)/cross-root"
TARGET_ROOT="$(realpath ..)/root"
TARGET=aarch64-elf
#GCCVERSION=11.2.0
GCCVERSION=master
BINUTILSVERSION=2.37

if [ -z "$MAKEFLAGS" ]; then
	MAKEFLAGS="$1"
fi
export MAKEFLAGS

rm -rf "$CROSS_ROOT"
rm -rf "$TARGET_ROOT"/usr/include
mkdir -p "$CROSS_ROOT"
export PATH="$CROSS_ROOT/bin:$PATH"



#mkdir -p ./cross-root/include/
#cp -r ../libc/include/* ../host/cross-root/include/

mkdir -p build-toolchain
cd build-toolchain

rm -rf build
mkdir -p build



rm -rf build-gcc build-binutils


mkdir build-binutils
cd build-binutils
../binutils-$BINUTILSVERSION/configure --target=$TARGET --prefix="$CROSS_ROOT" --with-sysroot="$TARGET_ROOT" --disable-werror
make -j16
make install

cd ../gcc-$GCCVERSION
contrib/download_prerequisites
#cd libstdc++-v3 && autoconf && cd ..
cd ..
mkdir build-gcc
cd build-gcc
../gcc-$GCCVERSION/configure --target=$TARGET --prefix="$CROSS_ROOT" --with-sysroot="$TARGET_ROOT" --enable-languages=c --disable-multilib
make all-gcc -j16
make install-gcc

make all-target-libgcc -j16
make install-target-libgcc

exit 0
