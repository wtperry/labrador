#pragma once

#include <bootloader/boot_spec.h>
#include <kernel/arch/paging.h>

boot_info* vmm_preinit(boot_info* info);
void vmm_init(boot_info* info);

vaddr_t vmm_alloc_region(size_t num_pages);