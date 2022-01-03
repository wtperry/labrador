#pragma once

#include <stdint.h>
#include <stddef.h>

#include <bootloader/boot_spec.h>
#include <kernel/arch/paging.h>

void pmm_init(mmap_entry* mmap, size_t mmap_entries);

size_t pmm_get_memory_size(void);
size_t pmm_get_free_memory(void);
size_t pmm_get_reserved_memory(void);
size_t pmm_get_used_memory(void);

paddr_t allocate_page(void);
void free_page(paddr_t page_phys_addr);