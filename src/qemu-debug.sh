#!/bin/sh
set -e
. ./iso.sh
 
qemu-system-$(./target-triplet-to-arch.sh $HOST) -s -S -cdrom labrador.iso &
gdb -ex 'target remote localhost:1234' -ex 'symbol-file kernel/labrador.kernel'