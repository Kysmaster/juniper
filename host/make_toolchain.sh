#!/usr/bin/env bash

set -e
set -x

CROSS_ROOT="$(pwd)/cross-root"
TARGET_ROOT="$(realpath ..)/root"
TARGET=aarch64-juniper
GCCVERSION=11.2.0
BINUTILSVERSION=2.37

if [ -z "$MAKEFLAGS" ]; then
	MAKEFLAGS="$1"
fi
export MAKEFLAGS

rm -rf "$CROSS_ROOT"
rm -rf "$TARGET_ROOT"/usr/include
mkdir -p "$CROSS_ROOT"
export PATH="$CROSS_ROOT/bin:$PATH"

if [ -x "$(command -v gmake)" ]; then
    mkdir -p "$CROSS_ROOT/bin"
    cat <<EOF >"$CROSS_ROOT/bin/make"
#!/usr/bin/env sh
gmake "\$@"
EOF
    chmod +x "$CROSS_ROOT/bin/make"
fi


cp -r ../libc/include ../host/cross-root/include
cp -r ../libc/include ../root/usr/include

mkdir -p build-toolchain
cd build-toolchain

rm -rf build
mkdir -p build



rm -rf gcc-$GCCVERSION binutils-$BINUTILSVERSION build-gcc build-binutils
if [ ! -f binutils-$BINUTILSVERSION.tar.gz ]; then
	wget https://ftp.gnu.org/gnu/binutils/binutils-$BINUTILSVERSION.tar.gz
fi
if [ ! -f gcc-$GCCVERSION.tar.gz ]; then
	wget https://ftp.gnu.org/gnu/gcc/gcc-$GCCVERSION/gcc-$GCCVERSION.tar.gz
fi
tar -xf gcc-$GCCVERSION.tar.gz
tar -xf binutils-$BINUTILSVERSION.tar.gz

cd binutils-$BINUTILSVERSION
patch -p1 < ../../binutils.patch
cd ..
mkdir build-binutils
cd build-binutils
../binutils-$BINUTILSVERSION/configure --target=$TARGET --prefix="$CROSS_ROOT" --with-sysroot="$TARGET_ROOT" --disable-werror
make -j16
make install

cd ../gcc-$GCCVERSION
contrib/download_prerequisites
patch -p1 < ../../gcc.patch
cd libstdc++-v3 && autoconf && cd ..
cd ..
mkdir build-gcc
cd build-gcc
../gcc-$GCCVERSION/configure --target=$TARGET --prefix="$CROSS_ROOT" --with-sysroot="$TARGET_ROOT" --enable-languages=c,c++ --disable-multilib
make all-gcc -j16
make install-gcc
cd ../..



cd build-toolchain
cd build-gcc
make all-target-libgcc -j16
make install-target-libgcc

exit 0
