IMAGE_NAME:=labrador

OVMF:=~/src/edk2/Build/OvmfX64/DEBUG_GCC5/FV/OVMF.fd
QEMU:=qemu-system-x86_64
QEMUFLAGS:=--bios $(OVMF) -M q35 -net none -cdrom $(IMAGE_NAME).iso -m 4G -boot d

MAKEFLAGS += --no-print-directory	

.PHONY: all
all: $(IMAGE_NAME).iso

.PHONY: run
run: $(IMAGE_NAME).iso
	@$(QEMU) $(QEMUFLAGS)

limine:
	@git clone https://github.com/limine-bootloader/limine.git --branch=v4.x-branch-binary --depth=1
	@$(MAKE) -C limine

.PHONY: kernel
kernel:
	$(MAKE) -C kernel

$(IMAGE_NAME).iso: limine kernel
	@rm -rf iso_root
	@mkdir -p iso_root
	@cp build/kernel/kernel.elf \
		limine.cfg limine/limine.sys limine/limine-cd.bin limine/limine-cd-efi.bin iso_root/
	@mkdir -p iso_root/EFI/BOOT
	@cp limine/BOOT*.EFI iso_root/EFI/BOOT/
	@xorriso -as mkisofs -b limine-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot limine-cd-efi.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		iso_root -o $(IMAGE_NAME).iso
	@limine/limine-deploy $(IMAGE_NAME).iso
	@rm -rf iso_root

.PHONY: clean
clean:
	@rm -rf iso_root $(IMAGE_NAME).iso
	@$(MAKE) -C kernel clean

.PHONY: distclean
distclean:
	@rm -rf limine