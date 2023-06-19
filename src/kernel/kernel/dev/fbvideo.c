#include <kernel/dev/fbvideo.h>

#include <kernel/log.h>

int fb_is_initialized = 0;

void *fb_ptr;
uint16_t fb_height;
uint16_t fb_width;
uint16_t fb_pitch;

int fb_init() {
    arch_fb_init();

    log_printf(LOG_DEBUG, "Framebuffer intialized at %llx, %ux%u with pitch %u", fb_ptr, fb_width, fb_height, fb_pitch);

    return 0;
}

int fb_get_info(struct fb_info *fb_info) {
    if (!fb_is_initialized) {
        return 1;
    }

    fb_info->fb = fb_ptr;
    fb_info->hres = fb_width;
    fb_info->vres = fb_height;
    fb_info->pitch = fb_pitch;

    return 0;
}