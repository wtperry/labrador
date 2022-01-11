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

	vfs_read(file, 0, 256, (uint8_t*)buff);

	printf("%.256s\n", buff);

	while (1) {
		asm("hlt");
	}
}