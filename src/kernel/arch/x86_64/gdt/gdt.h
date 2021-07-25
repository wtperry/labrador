#pragma once

#include <stdint.h>

#define GDT_PRESENT     (1 << 7)
#define GDT_RING0       (0 << 5)
#define GDT_RING3       (3 << 5)
#define GDT_CODE        (0b11 << 3)
#define GDT_DATA        (0b10 << 3)

#define GDT_FLAG_LONG   (1 << 5)

struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t limit_high;
    uint8_t base_high;
} __attribute__((packed));

struct gdt_ptr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

void gdt_init(void);