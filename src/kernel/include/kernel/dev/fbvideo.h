#pragma once

#include <stdint.h>

struct fb_info {
    void* fb;
    uint16_t vres;
    uint16_t hres;
    uint16_t pitch;
};

int arch_fb_init();

int fb_init();
int fb_get_info(struct fb_info *fb_info);