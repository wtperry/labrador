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
#include <kernel/tasking.h>
#include <kernel/arch/time.h>

thread_t *thread1;
thread_t *thread2;

void task1(void) {
	uint64_t next = get_time_from_boot();
	while (1) {
		next = next * 1103515245 + 12345;
		write_serial("A\r\n");
		uint64_t sleep_time = next % 100000;
		sleep(sleep_time);
	}
}

void task2(void) {
	uint64_t next = get_time_from_boot();
	while (1) {
		next = next * 1103515245 + 12345;
		write_serial("B\r\n");
		uint64_t sleep_time = next % 50000;
		sleep(sleep_time);
	}
}

extern boot_info *info;

void generic_early(void) {
	init_heap();
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

	init_serial();
	write_serial("Hello serial!\n\r");

	tasking_init();
}

void kernel_main(void) {
	thread1 = create_task(&task1);
	thread2 = create_task(&task2);
	
	while (1) {
		sleep(0xFFFFFFFF);
	}

	while (1) {
		asm("hlt");
	}
}