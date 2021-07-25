#include <stdio.h>
#include <stdint.h>
 
#include <kernel/tty.h>
#include <kernel/hal.h>
#include <kernel/heap.h>

#include "multiboot.h"

void print_ram_map(multiboot_info_t* mbd, uint32_t magic) {
	// Make sure the magic number matches for memory mapping
	if(magic != MULTIBOOT_BOOTLOADER_MAGIC) {
		printf("Invalid magic number!\n");
	}

	// Check bit 6 to see if we have a valid memory map
	if (!(mbd->flags >> 6 & 0x1)) {
		printf("Invalid memory map given by GRUB bootloader\n");
	}

	uint64_t total_ram = 0;

	// Loop through the memory map and display the values
	for(size_t i = 0; i < mbd->mmap_length; i += sizeof(multiboot_memory_map_t)) {
		multiboot_memory_map_t* mmmt = (multiboot_memory_map_t*) (mbd->mmap_addr + i);

		total_ram += mmmt->len;

		printf("Start Addr: %.16llx | Length: %.16llx | Size: %d | Type: %d\n", mmmt->addr, mmmt->len, mmmt->size, mmmt->type);

		if (mmmt->type == MULTIBOOT_MEMORY_AVAILABLE) {
			/* 
             * Do something with this memory block!
             * BE WARNED that some of memory shown as availiable is actually 
             * actively being used by the kernel! You'll need to take that
             * into account before writing to memory!
             */
		}
	}

	printf("Total RAM: %dMB\n", total_ram/1024/1024);
}
 
void kernel_main(multiboot_info_t* mbd, uint32_t magic) {
	terminal_initialize();
	printf("Hello, kernel World!\n");

	init_hal();

	print_ram_map(mbd, magic);

	print_time();
}