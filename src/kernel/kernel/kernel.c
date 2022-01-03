#include <kernel/tty.h>
#include <kernel/vmm.h>
#include <kernel/pmm.h>
#include <kernel/heap.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <bootloader/boot_spec.h>

const char* MemoryTypeStrings[] = {
	"Reserved",
	"Free",
	"ACPI",
	"MMIO",
	"Bootloader",
	"Efi Runtime",
	"Other"
};

void kernel_main(boot_info* info) {
	info = vmm_preinit(info);
	terminal_initialize(info->fb_ptr, info->fb_width, info->fb_height, info->fb_scanline, info->font);
	pmm_init(&info->mmap, info->num_mmap_entries);
	vmm_init();
	init_heap();

	/*
	mmap_entry* mmap = &info->mmap;
	for(size_t i = 0; i < info->num_mmap_entries; i++) {
		if (mmap[i].size >= 1024*1024*1024) {
			printf("%.16lx-%.16lx    %4dGB    %11s\n", mmap[i].address, mmap[i].address+mmap[i].size, mmap[i].size/1024/1024/1024, MemoryTypeStrings[mmap[i].type]);
		} else if (mmap[i].size >= 1024*1024) {
			printf("%.16lx-%.16lx    %4dMB    %11s\n", mmap[i].address, mmap[i].address+mmap[i].size, mmap[i].size/1024/1024, MemoryTypeStrings[mmap[i].type]);
		} else if (mmap[i].size >= 1024) {
			printf("%.16lx-%.16lx    %4dKB    %11s\n", mmap[i].address, mmap[i].address+mmap[i].size, mmap[i].size/1024, MemoryTypeStrings[mmap[i].type]);
		} else {
			printf("%.16lx-%.16lx    %4dB     %11s\n", mmap[i].address, mmap[i].address+mmap[i].size, mmap[i].size, MemoryTypeStrings[mmap[i].type]);
		}
	}
	*/

	printf("\n");
	printf("Total Memory: %dMB\n", pmm_get_memory_size()/1024/1024);
	printf("\n");

	for(size_t i = 0; i < 8; i++) {
		printf("%.16lx\n", kmalloc(1024));
	}

	printf("kmalloc test done!\n");

	while (1) {
		asm("hlt");
	}
}