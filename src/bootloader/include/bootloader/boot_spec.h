#pragma once

#include <stdint.h>
#include <stddef.h>
#include <bootloader/psf1.h>

#define BOOTLOADER_MAGIC "WOW!"

#define MMAP_RESERVED       0
#define MMAP_FREE           1
#define MMAP_ACPI           2
#define MMAP_MMIO           3
#define MMAP_LOADER         4
#define MMAP_UEFI_RUNTIME   5
#define MMAP_OTHER          6

#define MAX_BOOT_INFO_SIZE  2048    //2 KB

typedef struct {
    uint64_t address;
    size_t size;
    uint32_t type;
} mmap_entry;

typedef struct {
    char magic[4];
    void* fb_ptr;
    size_t fb_size;
    uint16_t fb_width;
    uint16_t fb_height;
    uint16_t fb_scanline;
    PSF1_FONT* font;
    void* rsdp;
    size_t num_mmap_entries;
    mmap_entry mmap;
} boot_info;