#include <kernel/hal.h>
#include <stdio.h>
#include "gdt/gdt.h"
#include "idt/idt.h"
#include "pic/pic.h"
#include "drivers/cmos/time.h"

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

void print_time() {
    struct date_time curr_time = get_cmos_time();

    printf("%02d/%02d/%02d%02d %d:%02d:%02d", curr_time.month, curr_time.day, curr_time.century, curr_time.year, curr_time.hour, curr_time.minute, curr_time.second);
}