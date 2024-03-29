#include <stdint.h>
#include <stddef.h>
#include <libk/stdio.h>
#include <libk/string.h>

#include <kernel/vmm.h>
#include <kernel/pmm.h>
#include <kernel/arch/paging.h>
#include "idt/exception.h"

vaddr_t curr_brk;

void remap_physical_memory(paddr_t PML4T) {
    // create pointer to first PML4E
    uint64_t* identity_PML4E = (uint64_t*)PML4T;

    // create pointer to new map location
    uint16_t PML4T_Index = (PHYSICAL_MAP_BASE >> 39) & 0x1ff;
    uint64_t* new_map_PML4E = (uint64_t*)PML4T + PML4T_Index;

    // move entry
    *new_map_PML4E = *identity_PML4E;
    *identity_PML4E = 0;
}

boot_info* fix_boot_info(boot_info* info) {
    info = (boot_info*)PHYS_TO_VIRT(info);

    info->fb_ptr = (void*)PHYS_TO_VIRT(info->fb_ptr);
    info->font = (PSF1_FONT*)PHYS_TO_VIRT(info->font);
    info->font->psf1_header = (PSF1_HEADER*)PHYS_TO_VIRT(info->font->psf1_header);
    info->font->glyph_buffer = (void*)PHYS_TO_VIRT(info->font->glyph_buffer);
    info->rsdp = (void*)PHYS_TO_VIRT(info->rsdp);

    return info;
}

paddr_t get_PML4T() {
    // get current PML4T from cr3
    paddr_t PML4T;
    asm("movq %%cr3, %0" : "=r" (PML4T));
    PML4T = PML4T & 0xFFFFFFFFFFFFF000;
    return PML4T;
}

void map_page(vaddr_t virt_addr, paddr_t phys_addr) {    
    paddr_t PML4T = get_PML4T();

    uint16_t PML4T_index = (virt_addr >> 39) & 0x1ff;
    uint16_t PDPT_index = (virt_addr >> 30) & 0x1ff;
    uint16_t PDT_index = (virt_addr >> 21) & 0x1ff;
    uint16_t PT_index = (virt_addr >> 12) & 0x1ff;

    uint64_t* PML4E = ((uint64_t*)PML4T + PML4T_index);
    paddr_t PDPT;
    if (!(*(uint64_t*)PHYS_TO_VIRT(PML4E) & PAGE_PRESENT)) {
        //No PDPT for this address, make one
        PDPT = allocate_page();
        //zero out the new PDPT
        memset((void*)PHYS_TO_VIRT(PDPT), 0, 4096);
        //add to the PML4E
        *(uint64_t*)PHYS_TO_VIRT(PML4E) = ((PDPT & PAGE_ADDR_MASK) | PAGE_PRESENT | PAGE_WRITEABLE);
    } else {
        PDPT = (*(uint64_t*)PHYS_TO_VIRT(PML4E) & PAGE_ADDR_MASK);
    }

    uint64_t* PDPE = ((uint64_t*)PDPT + PDPT_index);
    paddr_t PDT;
    if (!(*(uint64_t*)PHYS_TO_VIRT(PDPE) & PAGE_PRESENT)) {
        //No PDT for this address, make one
        PDT = allocate_page();
        //zero out the new PDT
        memset((void*)PHYS_TO_VIRT(PDT), 0, 4096);
        //add to the PDPE
        *(uint64_t*)PHYS_TO_VIRT(PDPE) = (((uint64_t)PDT & PAGE_ADDR_MASK) | PAGE_PRESENT | PAGE_WRITEABLE);
    } else {
        PDT = (*(uint64_t*)PHYS_TO_VIRT(PDPE) & PAGE_ADDR_MASK);
    }

    uint64_t* PDE = ((uint64_t*)PDT + PDT_index);
    paddr_t PT;
    if (!(*(uint64_t*)PHYS_TO_VIRT(PDE) & PAGE_PRESENT)) {
        //No PT for this address, make one
        PT = allocate_page();
        //zero out the new PT
        memset((void*)PHYS_TO_VIRT(PT), 0, 4096);
        //add to the PDE
        *(uint64_t*)PHYS_TO_VIRT(PDE) = (((uint64_t)PT & PAGE_ADDR_MASK) | PAGE_PRESENT | PAGE_WRITEABLE);
    } else {
        PT = (*(uint64_t*)PHYS_TO_VIRT(PDE) & PAGE_ADDR_MASK);
    }

    uint64_t* PTE = ((uint64_t*)PT + PT_index);
    *(uint64_t*)PHYS_TO_VIRT(PTE) = (((uint64_t)phys_addr & PAGE_ADDR_MASK) | PAGE_PRESENT | PAGE_WRITEABLE);

    memset((void*)virt_addr, 0, PAGE_SIZE);
}

uint64_t* get_pte(vaddr_t virt_addr) {
    paddr_t PML4T = get_PML4T();

    uint16_t PML4T_index = (virt_addr >> 39) & 0x1ff;
    uint16_t PDPT_index = (virt_addr >> 30) & 0x1ff;
    uint16_t PDT_index = (virt_addr >> 21) & 0x1ff;
    uint16_t PT_index = (virt_addr >> 12) & 0x1ff;

    uint64_t* PML4E = ((uint64_t*)PML4T + PML4T_index);
    paddr_t PDPT = (*(uint64_t*)PHYS_TO_VIRT(PML4E) & PAGE_ADDR_MASK);

    uint64_t* PDPE = ((uint64_t*)PDPT + PDPT_index);
    paddr_t PDT = (*(uint64_t*)PHYS_TO_VIRT(PDPE) & PAGE_ADDR_MASK);

    uint64_t* PDE = ((uint64_t*)PDT + PDT_index);
    paddr_t PT = (*(uint64_t*)PHYS_TO_VIRT(PDE) & PAGE_ADDR_MASK);

    uint64_t* PTE = ((uint64_t*)PT + PT_index);

    return (uint64_t)PHYS_TO_VIRT(PTE);
}

void page_fault_handler(struct interrupt_data* ex_data) {
    printf("Page Fault!\n");
    printf("Error code: %.16lx\n", ex_data->error_code);

    if (!(ex_data->error_code & 1)) {
        uint64_t cr2;

        asm volatile ("movq %%cr2, %0" : "=r"(cr2));

        printf("Page not present!\n");
        printf("Virt_Addr: %.16lx\n", cr2);

        if (cr2 && cr2 <= curr_brk) {
            printf("Address before brk\n");
            map_page((cr2 & 0xFFFFFFFFFFFFF000), allocate_page());
            return;
        }
    }

    for (;;) {
        asm("hlt");
    }
}

boot_info* vmm_preinit(boot_info* info) {
    paddr_t PML4T = get_PML4T();

    remap_physical_memory(PML4T);

    // fix pointers in boot info to map to remapped physical memory
    return fix_boot_info(info);
}

void vmm_init(boot_info* info) {
    curr_brk = (vaddr_t)info->first_free_page;

    register_exception_handler(page_fault_handler, PAGE_FAULT);
}

vaddr_t vmm_alloc_region(size_t num_pages) {
    vaddr_t old_brk = curr_brk;
    curr_brk = curr_brk + num_pages * 4096;
    return old_brk;
}