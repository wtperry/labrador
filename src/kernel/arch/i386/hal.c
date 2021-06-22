#include <kernel/hal.h>
#include "gdt/gdt.h"
#include "idt/idt.h"
#include "pic/pic.h"

void init_hal() {
    init_gdt();
    init_idt();
    init_pic();
}

void enable_interrupts() {
    asm volatile ("sti");
}

void disable_interrupts() {
    asm volatile ("cli");
}