#ifndef IDT_H
#define IDT_H

#include <stdint.h>

#define I386_MAX_INTERRUPTS 256

struct idt_entry {
    uint16_t base_low;
    uint16_t sel;
    uint8_t reserved;
    uint8_t flags;
    uint16_t base_high;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

void idt_init(void);

#endif