#pragma once

#include <bootloader/boot_spec.h>
#include <kernel/arch/paging.h>

void boot_parse(boot_info *info);
paddr_t boot_get_fb_addr();
uint16_t boot_get_fb_height();
uint16_t boot_get_fb_width();
uint16_t boot_get_fb_pitch();