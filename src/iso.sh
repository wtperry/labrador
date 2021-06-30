#!/bin/sh
set -e
. ./build.sh
 
mkdir -p isodir
mkdir -p isodir/boot
mkdir -p isodir/boot/grub
 
cp sysroot/boot/labrador.kernel isodir/boot/labrador.kernel
cat > isodir/boot/grub/grub.cfg << EOF
set timeout=0
menuentry "labrador" {
	multiboot /boot/labrador.kernel
}
EOF
grub-mkrescue -o labrador.iso isodir