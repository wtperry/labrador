#include <kernel/mem.h>

#include <kernel/log.h>
#include <kernel/string.h>
#include <kernel/heap.h>
#include <kernel/smp.h>
#include <kernel/ds/list.h>

#include <stddef.h>
#include <stdint.h>
#include <limine.h>

#define KERNEL_REGION_START ((void *)0xffffff8000000000)
#define KERNEL_REGION_END ((void *)0xffffffff80000000)
#define HEAP_START_PAGES 1

#define PML4T_INDEX(vaddr)  (((uintptr_t)vaddr >> 39) & 0x1ff)
#define PDPT_INDEX(vaddr)   (((uintptr_t)vaddr >> 30) & 0x1ff)
#define PDT_INDEX(vaddr)    (((uintptr_t)vaddr >> 21) & 0x1ff)
#define PT_INDEX(vaddr)     (((uintptr_t)vaddr >> 12) & 0x1ff)

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

list_t *kernel_vmem_free_list;

typedef struct {
    uintptr_t start;
    size_t num_pages;
} free_list_entry_t;

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

void dump_kernel_free_list() {
    log_printf(LOG_DEBUG, "Kernel address space freelist");
    for (list_node_t *node = kernel_vmem_free_list->head; node; node = node->next) {
        free_list_entry_t *entry = (free_list_entry_t *)node->value;
        log_printf(LOG_DEBUG, "Addr: %.16lx    Pages: %d", entry->start, entry->num_pages);
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

/// @brief Returns pointer to page table in virutal memory given an upper table entry
/// @param table_entry Entry in upper level page table
/// @return Pointer to page table in virtual memory
inline static union paging_entry_t *get_subtable_ptr(union paging_entry_t *table_entry) {
    return (union paging_entry_t *)phys_to_virt(table_entry->bits.base << 12);
}

/// @brief Returns page table entry corresponding to vaddr.
///        Allocates new entires in upper level tables as required.
/// @param pml4t PML4 table
/// @param vaddr Virtual address
/// @param flags Flags to assign to upper level table entries
/// @return Page table entry coresponding to given virtual address
static union paging_entry_t *get_pte(union paging_entry_t *pml4t, uintptr_t vaddr, uint64_t flags) {
    union paging_entry_t *pml4e = pml4t + PML4T_INDEX(vaddr);

    if (!pml4e->bits.present) {
        mem_map_page(pml4e, mem_get_phys_page(), flags);
    }

    union paging_entry_t *pdpe = get_subtable_ptr(pml4e) + PDPT_INDEX(vaddr);

    if (!pdpe->bits.present) {
        mem_map_page(pdpe, mem_get_phys_page(), flags);
    }

    union paging_entry_t *pdte = get_subtable_ptr(pdpe) + PDT_INDEX(vaddr);

    if (!pdte->bits.present) {
        mem_map_page(pdte, mem_get_phys_page(), flags);
    }

    union paging_entry_t *pte = get_subtable_ptr(pdte) + PT_INDEX(vaddr);

    return pte;
}

static void vmm_init() {
    union paging_entry_t *pml4t = get_current_pml4();
    this_core->current_pml4 = pml4t;

    //Kickstart the heap with the first part of the kernel object region.
    //We have to do this first, as the virtual allocator will require heap
    //objects to track virtual memory regions

    for (size_t i = 0; i < HEAP_START_PAGES; i++) {
        uintptr_t vaddr = (uintptr_t)KERNEL_REGION_START + i * PAGE_SIZE;
        union paging_entry_t *pte = get_pte(pml4t, vaddr, MEM_FLAGS_WRITEABLE);
        mem_map_page(pte, mem_get_phys_page(), MEM_FLAGS_WRITEABLE);
    }

    heap_init(KERNEL_REGION_START, HEAP_START_PAGES);

    kernel_vmem_free_list = list_create();
    free_list_entry_t *first_free = kmalloc(sizeof(*first_free));
    first_free->start = (uintptr_t)KERNEL_REGION_START + HEAP_START_PAGES * PAGE_SIZE;
    first_free->num_pages = ((uintptr_t)KERNEL_REGION_END - (uintptr_t)KERNEL_REGION_START) / PAGE_SIZE - HEAP_START_PAGES;
    list_append(kernel_vmem_free_list, first_free);

    log_printf(LOG_DEBUG, "Heap starts at pml4 index %d, pdp index %d, pdt index %d, and pt index %d", PML4T_INDEX(KERNEL_REGION_START),
        PDPT_INDEX(KERNEL_REGION_START), PDT_INDEX(KERNEL_REGION_START), PT_INDEX(KERNEL_REGION_START));
    log_printf(LOG_DEBUG, "Added entry to free list starting at %.16lx with %ld pages", first_free->start, first_free->num_pages);
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
    list_node_t *node = kernel_vmem_free_list->head;

    uintptr_t region_start = 0;

    while (node) {
        free_list_entry_t *free_list_entry = node->value;
        if (free_list_entry->num_pages > num_pages) {
            //replace free list entry with a new smaller one
            free_list_entry->num_pages -= num_pages;
            region_start = free_list_entry->start;
            free_list_entry->start += num_pages * PAGE_SIZE;
            break;
        } else if (free_list_entry->num_pages == num_pages) {
            //we've found what we need
            region_start = free_list_entry->start;
            list_remove(kernel_vmem_free_list, node);
            break;
        }

        node = node->next;
    }

    if (!region_start) {
        //nothing available!
        return NULL;
    }

    for (size_t i = 0; i < num_pages; i++) {
        uintptr_t vaddr = region_start + i * PAGE_SIZE;
        union paging_entry_t *pte = get_pte(this_core->current_pml4, vaddr, flags);
        mem_map_page(pte, mem_get_phys_page(), flags);
    }

    return (void *)region_start;
}

void mem_free_kernel_region(void *vaddr, size_t num_pages) {
    //Loop through free list to find where to insert
    list_node_t *node = kernel_vmem_free_list->head;

    while (node) {
        free_list_entry_t *free_list_entry = node->value;
        if (free_list_entry->start > (uintptr_t)vaddr) {
            //we need to insert before this node
            break;
        }

        node = node->next;
    }

    free_list_entry_t *new_entry = kmalloc(sizeof(*new_entry));
    new_entry->start = (uintptr_t)vaddr;
    new_entry->num_pages = num_pages;
    list_node_t *new_node = list_insert_before(kernel_vmem_free_list, node, new_entry);

    //check next and previous to see if we can combine
    list_node_t *prev_node = new_node->prev;
    if (prev_node) {
        free_list_entry_t *prev_entry = (free_list_entry_t *)prev_node->value;
        if ((prev_entry->start + prev_entry->num_pages * PAGE_SIZE) == (uintptr_t)vaddr) {
            prev_entry->num_pages += num_pages;
            list_remove(kernel_vmem_free_list, new_node);
            new_node = prev_node;
            new_entry = prev_entry;
        }
    }

    list_node_t *next_node = new_node->next;
    if (next_node) {
        free_list_entry_t *next_entry = (free_list_entry_t *)next_node->value;
        if (((uintptr_t)vaddr + num_pages * PAGE_SIZE) == next_entry->start) {
            new_entry->num_pages += next_entry->num_pages;
            list_remove(kernel_vmem_free_list, next_node);
        }
    }
}