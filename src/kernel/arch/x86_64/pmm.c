#include <kernel/pmm.h>
#include <stddef.h>
#include <stdint.h>
#include "paging.h"

size_t memory_size;
size_t used_memory;
size_t reserved_memory;
size_t free_memory;

void* next_free_page;

typedef struct {
    void* next;
} free_page_header;

void pmm_init(mmap_entry* mmap, size_t mmap_entries) {
    memory_size = 0;
    used_memory = 0;
    reserved_memory = 0;
    free_memory = 0;

    next_free_page = NULL;

    for (size_t i = 0; i < mmap_entries; i++) {
        memory_size += mmap[i].size;

        switch (mmap[i].type)
        {
        case MMAP_FREE:
            free_memory += mmap[i].size;
            for (size_t j = 0; j < mmap[i].size; j += 4096) {
                if ((uint64_t)mmap[i].address + j != 0) {
                    free_page((void*)(mmap[i].address + j));
                }
            }
            break;

        case MMAP_LOADER:
            used_memory += mmap[i].size;
            break;
        
        case MMAP_RESERVED:
        case MMAP_ACPI:
        case MMAP_MMIO:
        case MMAP_UEFI_RUNTIME:
        case MMAP_OTHER:
        default:
            reserved_memory += mmap[i].size;
            break;
        }
    }
}

size_t pmm_get_memory_size() {
    return memory_size;
}

size_t pmm_get_free_memory() {
    return free_memory;
}

size_t pmm_get_reserved_memory() {
    return reserved_memory;
}

size_t pmm_get_used_memory() {
    return used_memory;
}

void* allocate_page() {
    free_memory -= PAGE_SIZE;
    used_memory += PAGE_SIZE;

    void* page = next_free_page;
    next_free_page = ((free_page_header*)PHYS_TO_VIRT(next_free_page))->next;
    return page;
}

void free_page(void* page_phys_addr) {
    free_memory += PAGE_SIZE;
    used_memory -= PAGE_SIZE;

    ((free_page_header*)PHYS_TO_VIRT(page_phys_addr))->next = next_free_page;
    next_free_page = page_phys_addr;
}