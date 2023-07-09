#include <kernel/dev/serial.h>
#include <kernel/log.h>
#include <kernel/idt.h>
#include <kernel/gdt.h>
#include <kernel/mem.h>
#include <kernel/smp.h>

#include <stdint.h>
#include <stddef.h>
#include <limine.h>

// The Limine requests can be placed anywhere, but it is important that
// the compiler does not optimise them away, so, usually, they should
// be made volatile or equivalent.

static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

// Halt and catch fire function.
static void hcf(void) {
    asm ("cli");
    for (;;) {
        asm ("hlt");
    }
}

// The following will be our kernel's entry point.
// If renaming _start() to something else, make sure to change the
// linker script accordingly.
void _start(void) {
    serial_init();
    log_init();
    log_add_output(LOG_DEBUG, serial_write);
    log_printf(LOG_INFO, "Kernel log initialized");

    // Ensure we got a framebuffer.
    if (framebuffer_request.response == NULL
     || framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }

    // Fetch the first framebuffer.
    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];
    log_printf(LOG_DEBUG, "Framebuffer found at: %.16lx", framebuffer->address);

    // Note: we assume the framebuffer model is RGB with 32-bit pixels.
    for (size_t i = 0; i < 100; i++) {
        uint32_t *fb_ptr = framebuffer->address;
        fb_ptr[i * (framebuffer->pitch / 4) + i] = 0xffffff;
    }

    gdt_init();
    idt_init();

    mem_init();

    // We're done, just hang...
    hcf();
}