mkdir -p ../build/newlib
cd ../build/newlib
../../src/newlib/configure --prefix=/usr --target=x86_64-labrador
make all
make DESTDIR="$HOME/labrador/src/sysroot" install
mv ../../src/sysroot/usr/x86_64-labrador/* ../../src/sysroot/usr
rmdir ../../src/sysroot/usr/x86_64-labrador