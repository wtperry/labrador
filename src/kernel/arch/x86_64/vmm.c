#include <stdint.h>
#include <stddef.h>

#include <kernel/vmm.h>
#include <kernel/pmm.h>
#include "paging.h"

void remap_physical_memory(void* PML4T) {
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

boot_info* vmm_preinit(boot_info* info) {
    // get current PML4T from cr3
    void* PML4T;
    asm("movq %%cr3, %0" : "=r" (PML4T));
    PML4T = (void*)((uint64_t)PML4T & 0xFFFFFFFFFFFFF000);

    remap_physical_memory(PML4T);

    // fix pointers in boot info to map to remapped physical memory
    return fix_boot_info(info);
}

void vmm_init(void) {

}