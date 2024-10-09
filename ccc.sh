GCC="gcc-14.2.0"
BINUTILS="binutils-2.43"
PREFIX="$HOME/opt/cross"
TARGET="x86_64-elf"
BASE=$(pwd)
export PATH="$PATH:$PREFIX/bin"

set -e

mkdir -p $PREFIX

wget https://ftp.gnu.org/gnu/gcc/$GCC/$GCC.tar.xz
wget https://ftp.gnu.org/gnu/binutils/$BINUTILS.tar.xz

xz -d $GCC.tar.xz
xz -d $BINUTILS.tar.xz

tar -xf $GCC.tar
tar -xf $BINUTILS.tar

cd $BINUTILS && mkdir build && cd build
../configure --target="$TARGET" --prefix="$PREFIX" --with-sysroot --disable-nls -disable-werror
make -j $(nproc)
make install
cd $BASE

cd $GCC && mkdir build && cd build
../configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers
make all-gcc -j $(nproc)
make all-target-libgcc -j $(nproc)
make install-gcc
make install-target-libgcc

# cleanup
cd $BASE
rm -rf $GCC $GCC.tar.xz $GCC.tar $BINUTILS $BINUTILS.tar.xz $BINUTILS.tar


echo "add $PREFIX/bin to your PATH"