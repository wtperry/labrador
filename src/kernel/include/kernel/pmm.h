#pragma once

#include <stdint.h>
#include <stddef.h>

#include <bootloader/boot_spec.h>

void pmm_init(mmap_entry* mmap, size_t mmap_entries);

size_t pmm_get_memory_size(void);
size_t pmm_get_free_memory(void);
size_t pmm_get_reserved_memory(void);
size_t pmm_get_used_memory(void);

void* allocate_page(void);
void lock_page(void* page_phys_addr);
void free_page(void* page_phys_addr);