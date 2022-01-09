#include <kernel/tty.h>
#include <kernel/vmm.h>
#include <kernel/pmm.h>
#include <kernel/heap.h>
#include <kernel/fs/vfs.h>
#include <kernel/fs/tmpfs.h>
#include <kernel/fs/tarfs.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
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
	vmm_init(info);
	init_heap();

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

	printf("\n");
	printf("Total Memory: %dMB\n", pmm_get_memory_size()/1024/1024);
	printf("\n");

	for(size_t i = 0; i < 8; i++) {
		printf("%.16lx\n", kmalloc(1024));
	}

	printf("New VMM block: %.16lx\n", vmm_alloc_region(6));

	for(size_t i = 0; i < 8; i++) {
		printf("%.16lx\n", kmalloc(1024));
	}

	printf("kmalloc test done!\n");

	vfs_init();
	tmpfs_install();
	tarfs_install();

	if (vfs_mount("tmpfs", "[rootfs]", "/")) {
		printf("Mount Failed!\n");
		while (1) {
			asm("hlt");
		}
	}

	fs_node_t* root_dir = vfs_get_fs_node("/");

	vfs_mkdir("/test");
	vfs_mkdir("/wazzup");
	printf("%i\n", vfs_create("/wazzup/test.txt"));

	const char msg[] = "Hello filesystem world!";

	fs_node_t* node = vfs_get_fs_node("/wazzup/test.txt");

	vfs_write(node, 0, strlen(msg) + 1, msg);

	char msg_out[256];

	vfs_read(node, 0, 256, msg_out);

	printf("%s\n", msg_out);

	size_t index = 0;
	dirent_t* dirent = vfs_readdir(root_dir, index);

	while(dirent) {
		printf("%s\n", dirent->name);
		kfree(dirent);
		dirent = vfs_readdir(root_dir, ++index);
	}

	printf("%.16lx    %lx\n", info->initrd, info->initrd_size);

	printf("Total Memory Used: %liKB\n", pmm_get_used_memory()/1024);

	while (1) {
		asm("hlt");
	}
}