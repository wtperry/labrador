#include <kernel/mem.h>

#include <kernel/log.h>
#include <kernel/string.h>
#include <kernel/heap.h>
#include <kernel/smp.h>

#include <stddef.h>
#include <stdint.h>
#include <limine.h>

#define KERNEL_REGION_START ((void *)0xffffff0000000000)
#define KERNEL_REGION_END ((void *)0xffffffff80000000)
#define HEAP_START_PAGES 1

static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0
};

static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};

size_t used_memory = 0;
size_t free_memory = 0;
size_t reserved_memory = 0;

paddr_t next_free_page = 0;

uintptr_t virt_offset = 0;  //offset for the higher half direct memory map provided by limine

typedef struct {
    paddr_t next;
} free_page_header;

static void log_memmap_entries(struct limine_memmap_entry **memmap_entries, size_t num_entries) {
    log_printf(LOG_DEBUG, "Memory map from bootloader:");
    log_printf(LOG_DEBUG, "%-16s  %-16s  Type", "Base", "Size");
    for (size_t i = 0; i < num_entries; i++) {
        struct limine_memmap_entry *memmap_entry = (*memmap_entries) + i;
        switch (memmap_entry->type) {
        case LIMINE_MEMMAP_USABLE:
            log_printf(LOG_DEBUG, "%.16lx  %.16lx  Usable", memmap_entry->base, memmap_entry->length);
            break;
        case LIMINE_MEMMAP_RESERVED:
            log_printf(LOG_DEBUG, "%.16lx  %.16lx  Reserved", memmap_entry->base, memmap_entry->length);
            break;
        case LIMINE_MEMMAP_ACPI_RECLAIMABLE:
            log_printf(LOG_DEBUG, "%.16lx  %.16lx  ACPI Reclaimable", memmap_entry->base, memmap_entry->length);
            break;
        case LIMINE_MEMMAP_ACPI_NVS:
            log_printf(LOG_DEBUG, "%.16lx  %.16lx  ACPI NVS", memmap_entry->base, memmap_entry->length);
            break;
        case LIMINE_MEMMAP_BAD_MEMORY:
            log_printf(LOG_DEBUG, "%.16lx  %.16lx  Bad Memory", memmap_entry->base, memmap_entry->length);
            break;
        case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE:
            log_printf(LOG_DEBUG, "%.16lx  %.16lx  Bootloader Reclaimable", memmap_entry->base, memmap_entry->length);
            break;
        case LIMINE_MEMMAP_KERNEL_AND_MODULES:
            log_printf(LOG_DEBUG, "%.16lx  %.16lx  Kernel/Modules", memmap_entry->base, memmap_entry->length);
            break;
        case LIMINE_MEMMAP_FRAMEBUFFER:
            log_printf(LOG_DEBUG, "%.16lx  %.16lx  Framebuffer", memmap_entry->base, memmap_entry->length);
            break;
        }
    }
}

static inline union paging_entry_t *get_current_pml4() {
    uintptr_t PML4T;
    asm("movq %%cr3, %0" : "=r" (PML4T));
    PML4T = PML4T & 0xFFFFFFFFFFFFF000;
    return (union paging_entry_t *)PML4T;
}

static inline void *phys_to_virt(paddr_t phys_addr) {
    return (void *)(phys_addr + virt_offset);
}

static void pmm_init() {
    if (hhdm_request.response == NULL) {
        log_printf(LOG_FATAL, "Bootloader memory map not found!");

        for(;;) {
            asm("hlt");
        }
    }

    virt_offset = hhdm_request.response->offset;

    if (memmap_request.response == NULL) {
        log_printf(LOG_FATAL, "Bootloader memory map not found!");

        for(;;) {
            asm("hlt");
        }
    }

    log_memmap_entries(memmap_request.response->entries, memmap_request.response->entry_count);

    struct limine_memmap_entry *memmap_entries = *(memmap_request.response->entries);

    for (size_t i = 0; i < memmap_request.response->entry_count; i++) {
        switch(memmap_entries[i].type) {
        case LIMINE_MEMMAP_USABLE:
            //Count these pages as used memory, as mem_free_page will
            //sub from used mem and add to free mem
            used_memory += memmap_entries[i].length;
            for (size_t j = 0; j < memmap_entries[i].length; j += PAGE_SIZE) {
                mem_free_phys_page(memmap_entries[i].base + j);
            }
            break;
            
        case LIMINE_MEMMAP_KERNEL_AND_MODULES:
        case LIMINE_MEMMAP_FRAMEBUFFER:
            used_memory += memmap_entries[i].length;
            break;
        
        default:
            reserved_memory += memmap_entries[i].length;
            break;
        }
    }

    log_printf(LOG_INFO, "Total Memory: %uMB", (free_memory + reserved_memory + reserved_memory)/1024/1024);
}

void vmm_init() {
    this_core->current_pml4 = get_current_pml4();

    //Kickstart the heap with the first part of the kernel object region.
    //We have to do this first, as the virtual allocator will require heap
    //objects to track virtual memory regions

    //TODO: Map pages into heap area

    heap_init(KERNEL_REGION_START, HEAP_START_PAGES);

    //TODO: Setup free list to use for kernel regions
}

void mem_init() {
    pmm_init();
    vmm_init();
}

void mem_free_phys_page(paddr_t page_addr) {
    free_memory += PAGE_SIZE;
    used_memory -= PAGE_SIZE;

    ((free_page_header *)phys_to_virt(page_addr))->next = next_free_page;
    next_free_page = page_addr;
}

paddr_t mem_get_phys_page() {
    free_memory -= PAGE_SIZE;
    used_memory -= PAGE_SIZE;

    paddr_t page = next_free_page;
    if (!page) {
        log_printf(LOG_FATAL, "Out of memory");
        for (;;) {
            asm ("hlt");
        }
    }

    next_free_page = ((free_page_header *)phys_to_virt(page))->next;
    memset(phys_to_virt(page), 0, PAGE_SIZE);

    return page;
}

void mem_map_page(union paging_entry_t *page_table_entry, paddr_t phys_addr, uint64_t flags) {
    page_table_entry->raw = 0;
    page_table_entry->bits.base = phys_addr >> 12;
    page_table_entry->bits.present = 1;

    if (!(flags & MEM_FLAGS_EXECUTABLE)) {
        page_table_entry->bits.no_execute = 1;
    }

    if (flags & MEM_FLAGS_USER) {
        page_table_entry->bits.user = 1;
    }

    if (flags & MEM_FLAGS_WRITEABLE) {
        page_table_entry->bits.writeable = 1;
    }
}

void mem_unmap_page(union paging_entry_t *page_table_entry) {
    page_table_entry->raw = 0;
}

void *mem_alloc_kernel_region(size_t num_pages, uint64_t flags) {

}

void *mem_free_kernel_region(void *virt_addr, size_t num_pages) {

}