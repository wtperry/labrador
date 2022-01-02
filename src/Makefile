QEMULOG=debug.log
OVMF=~/src/edk2/Build/OvmfX64/DEBUG_GCC5/FV/OVMF.fd
QEMU=qemu-system-x86_64
QEMUFLAGS=--bios $(OVMF) -net none -cdrom cdimage.iso -debugcon file:$(QEMULOG) -global isa-debugcon.iobase=0x402 -m 4G

.PHONY: all clean binaries run debug install-headers

all: cdimage.iso

binaries: install-headers
	@$(MAKE) -C libc
	@$(MAKE) -C bootloader
	@$(MAKE) -C kernel

cdimage.iso: binaries
	@dd if=/dev/zero of=fat.img bs=1k count=1440
	@mformat -i fat.img -f 1440 ::
	@mmd -i fat.img ::/EFI
	@mmd -i fat.img ::/EFI/BOOT
	@mmd -i fat.img ::/Labrador
	@mcopy -i fat.img bootloader/BOOTX64.EFI ::/EFI/BOOT
	@mcopy -i fat.img kernel/kernel.elf ::/Labrador
	@mcopy -i fat.img kernel/zap-light16.psf ::/Labrador
	@mkdir -p iso
	@cp fat.img iso
	@xorriso -as mkisofs -R -f -e fat.img -no-emul-boot -o cdimage.iso iso

run: cdimage.iso
	$(QEMU) $(QEMUFLAGS) &

debug: cdimage.iso
	LINE=$$(cat debug.log | awk '/BOOTX64.EFI/,0' | grep -m1 'Loading'); \
	BASE=$$(echo $$LINE | cut -d " " -f4); \
	ADDR=$$(readpe -H bootloader/BOOTX64.EFI | grep 'Address of .text section:' | tr ' ' '\n' | tail -n1); \
	TEXT=$$(python -c "print(hex($$BASE+$$ADDR))"); \
	echo "add-symbol-file bootloader/BOOTX64.EFI $$TEXT" > gdbscript; \
	echo "add-symbol-file kernel/kernel.elf" >> gdbscript
	$(QEMU) $(QEMUFLAGS) -s -S &
	gdb -ex 'source gdbscript' -ex 'target remote localhost:1234'

install-headers:
	@$(MAKE) -C libc install-headers
	@$(MAKE) -C bootloader install-headers
	@$(MAKE) -C kernel install-headers

clean:
	@$(MAKE) -C bootloader clean
	@$(MAKE) -C kernel clean
	@$(MAKE) -C libc clean
	@rm -f -r /sysroot
	@rm -f fat.img
	@rm -f cdimage.iso
	@rm -f -r iso
	@rm -f gdbscript
	@rm -f debug.log