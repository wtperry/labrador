#pragma once

#include <bootloader/boot_spec.h>

boot_info* vmm_preinit(boot_info* info);
void vmm_init(void);

void* kbrk(size_t size);