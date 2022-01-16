#include <kernel/tty.h>
#include <kernel/vmm.h>
#include <kernel/pmm.h>
#include <kernel/heap.h>
#include <kernel/dev/ramdisk.h>
#include <kernel/dev/serial.h>
#include <kernel/fs/vfs.h>
#include <kernel/fs/tmpfs.h>
#include <kernel/fs/tarfs.h>
#include <kernel/fs/devfs.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <bootloader/boot_spec.h>

#include <kernel/arch/acpi.h>
#include <kernel/arch/apic.h>
#include <kernel/arch/ps2.h>
#include <kernel/arch/cpu.h>
#include <kernel/arch/pit.h>

void ls(fs_node_t* node, size_t nested, const char* path) {
	size_t i = 2;
	dirent_t* dirent;

	while ((dirent = vfs_readdir(node, i++))) {
		for (size_t j = 0; j < nested; j++) {
			printf("  ");
		}

		printf("%s\n", dirent->name);

		char new_path[256];
		strcpy(new_path, path);
		strncat(new_path, "/", 256);
		strncat(new_path, dirent->name, 256);

		fs_node_t* child = vfs_get_fs_node(new_path);
		ls(child, nested + 1, new_path);
		kfree(child);
		kfree(dirent);
	}
}

void kernel_main(boot_info* info) {
	info = vmm_preinit(info);
	terminal_initialize(info->fb_ptr, info->fb_width, info->fb_height, info->fb_scanline, info->font);
	pmm_init(&info->mmap, info->num_mmap_entries);
	vmm_init(info);
	init_heap();

	init_serial();
	write_serial("Hello serial!\n\r");

	vfs_init();
	tmpfs_install();
	tarfs_install();

	if (vfs_mount("tmpfs", "[rootfs]", "/")) {
		printf("Mount Failed!\n");
		while (1) {
			asm("hlt");
		}
	}

	vfs_mkdir("/dev");
	devfs_install();

	printf("%.16lx    %lx\n", info->initrd, info->initrd_size);

	printf("Total Memory Used: %liKB\n", pmm_get_used_memory()/1024);

	ramdisk_init();
	create_ramdisk_from_address(info->initrd, info->initrd_size, RAMDISK_READ_ONLY, "ram0");

	vfs_mkdir("/boot");
	if (vfs_mount("tar", "/dev/ram0", "/boot")) {
		printf("Initrd Mount Failed!\n");
		while (1) {
			asm("hlt");
		}
	}

	ls(vfs_get_fs_node("/"), 0, "");

	char buff[256];
	fs_node_t* file = vfs_get_fs_node("/boot/test123/wow/file.txt");

	size_t file_size = vfs_read(file, 0, 256, (uint8_t*)buff);
	buff[file_size] = '\0';

	printf("%.256s\n", buff);

	struct RSDP* rsdp = info->rsdp;

	printf("Signature: %.8s\n", rsdp->signature);
	printf("OEM: %.6s\n", rsdp->oem_id);
	printf("Revision: %u\n", rsdp->revision);

	if (rsdp->revision == 2) {
		struct RSDP2* rsdp2 = (struct RSDP2*)rsdp;
		printf("Length: %u\n", rsdp2->length);
		printf("XSDT Address %.16lx\n", rsdp2->xsdt_addr);
		
		uint8_t sum = 0;
		for (int i = 0; i < sizeof(struct RSDP2); i++) {
			sum += ((uint8_t*)rsdp)[i];
		}

		if (sum == 0) {
			printf("RSDP checksum validated!\n\n");
		} else {
			printf("RSDP checksum failed!\n\n");
		}

		struct xsdt* xsdt = (struct xsdt*)PHYS_TO_VIRT(rsdp2->xsdt_addr);
		printf("Signature: %.4s\n", xsdt->hdr.signature);
		printf("Length: %u\n", xsdt->hdr.length);
		printf("Revision: %u\n", xsdt->hdr.revision);
		printf("OEM: %.6s\n", xsdt->hdr.oem_id);
		printf("OEM Table ID: %.8s\n", xsdt->hdr.oem_table_id);
		int xsdt_entries = (xsdt->hdr.length - sizeof(xsdt->hdr)) / 8;
		printf("Entries: %u\n", xsdt_entries);

		for (int i = 0; i < xsdt_entries; i++) {
			struct sdt_hdr* hdr = (struct sdt_hdr*)PHYS_TO_VIRT(xsdt->sdt_ptrs[i]);
			printf("Signature: %.4s\n", hdr->signature);
			if (!memcmp(hdr->signature, "APIC", 4)) {
				struct madt* madt = (struct madt*)hdr;
				printf("Local APIC Address: %.8x    Flags: %u\n", madt->lapic_addr, madt->flags);
				size_t num_apics = 0;
				struct madt_entry* entry = (struct madt_entry*)(madt->entries);
				while (((uint64_t)entry - (uint64_t)hdr) < hdr->length) {
					printf("APIC entry type: %u, length %u\n", entry->type, entry->length);
					if (entry->type == 0) {
						num_apics++;
						struct madt_entry_0* proc = (struct madt_entry_0*)entry;
						printf("Proc ID: %u    APIC ID: %u    Flags: %u\n", proc->proc_id, proc->apic_id, proc->flags);
					} else if (entry->type == 1) {
						struct madt_entry_1* ioapic = (struct madt_entry_1*)entry;
						printf("IO APIC ID: %u    IO APIC Addr: %.16lx    Interrupt Base: %x\n", ioapic->ioapic_id, ioapic->ioapic_addr, ioapic->int_base);
					} else if (entry->type == 2) {
						struct madt_entry_2* src = (struct madt_entry_2*)entry;
						printf("Bus: %u    IRQ: %u    Interrupt Num: %x    Flags %x\n", src->bus, src->irq, src->int_num, src->flags);
					}

					entry = (struct madt_entry*)((uint64_t)entry + entry->length);
				}

				printf("%u processors found!\n", num_apics);
			}
		}
	}

	apic_init();
	//ps2_init();
	enable_interrupts();
	pit_init();

	while (1) {
		asm("hlt");
	}
}