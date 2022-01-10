#pragma once

#include <stddef.h>

#define RAMDISK_READ_ONLY 0x1

void ramdisk_init(void);
int create_ramdisk_from_address(void* address, size_t size, unsigned int flags, const char* name);
void create_new_ramdisk(void);