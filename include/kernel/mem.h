#pragma once

#include <stdint.h>
#include <stddef.h>

#define PAGE_SIZE 4096

typedef uintptr_t paddr_t;

union paging_entry_t {
    struct {
        uint64_t present:1;
        uint64_t writeable:1;
        uint64_t user:1;
        uint64_t writethrough:1;
        uint64_t not_cacheable:1;
        uint64_t accessed:1;
        uint64_t dirty:1;
        uint64_t pat:1;
        uint64_t global:1;
        uint64_t available_low:3;
        uint64_t base:40;
        uint64_t available_high:7;
        uint64_t pke:4;
        uint64_t no_execute:1;
    } bits;

    uint64_t raw;
};

#define MEM_FLAGS_WRITEABLE     (1 << 0)
#define MEM_FLAGS_USER          (1 << 1)
#define MEM_FLAGS_EXECUTABLE    (1 << 2)

void mem_init();
void mem_free_phys_page(paddr_t page_addr);
paddr_t mem_get_phys_page();

void mem_map_page(union paging_entry_t *page_table_entry, paddr_t phys_addr, uint64_t flags);
void mem_unmap_page(union paging_entry_t *page_table_entry);
void *mem_alloc_kernel_region(size_t num_pages, uint64_t flags);
void mem_free_kernel_region(void *virt_addr, size_t num_pages);