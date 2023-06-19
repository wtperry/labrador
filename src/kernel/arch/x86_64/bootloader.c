#include "bootloader.h"

#include <bootloader/boot_spec.h>
#include <kernel/arch/paging.h>

paddr_t boot_fb_addr;
uint16_t boot_fb_height;
uint16_t boot_fb_width;
uint16_t boot_fb_pitch;

void boot_parse(boot_info *info) {
    boot_fb_addr = info->fb_ptr;
    boot_fb_width = info->fb_width;
    boot_fb_height = info->fb_height;
    boot_fb_pitch = info->fb_scanline;
}

paddr_t boot_get_fb_addr() {
    return boot_fb_addr;
}

uint16_t boot_get_fb_height() {
    return boot_fb_height;
}

uint16_t boot_get_fb_width() {
    return boot_fb_width;
}

uint16_t boot_get_fb_pitch() {
    return boot_fb_pitch;
}