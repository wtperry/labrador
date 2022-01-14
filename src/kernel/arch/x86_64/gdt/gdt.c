#include <stdint.h>

#define GDT_RING0_CODE  0x00A09A0000000000
#define GDT_RING0_DATA  0x00A0920000000000
#define GDT_RING3_CODE  0x00A0FA0000000000
#define GDT_RING3_DATA  0x00A0F20000000000

struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t limit_high;
    uint8_t base_high;
} __attribute__((packed));

struct gdt_entry_high {
    uint32_t base_highest;
    uint32_t reserved0;
};

struct gdt_ptr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

struct gdt {
    uint64_t entries[5];
    struct gdt_entry tss_low;
    struct gdt_entry_high tss_high;
}; __attribute__((packed));

typedef struct tss_entry {
	uint32_t reserved_0;
	uint64_t rsp[3];
	uint64_t reserved_1;
	uint64_t ist[7];
	uint64_t reserved_2;
	uint16_t reserved_3;
	uint16_t iomap_base;
} __attribute__ ((packed));

struct gdt_ptr gdt_ptr;
struct gdt gdt = {
    {
        0x0000000000000000,
        GDT_RING0_CODE,
        GDT_RING0_DATA,
        GDT_RING3_CODE,
        GDT_RING3_DATA
    },
    {0x0000, 0x0000, 0x00, 0xE9, 0x00, 0x00},
    {0x00000000, 0x00000000}
};

struct tss_entry tss = {0, {0, 0, 0}, 0, {0, 0, 0, 0, 0, 0, 0}, 0, 0, 0};

extern void load_gdt(uint64_t);

void init_gdt() {
    uint64_t tss_addr = (uint64_t)&tss;
    gdt.tss_low.base_low = tss_addr & 0xFFFF;
    gdt.tss_low.base_middle = (tss_addr >> 16) & 0xFF;
    gdt.tss_low.base_high = (tss_addr >> 24) & 0xFF;
    gdt.tss_high.base_highest = (tss_addr >> 32) & 0xFFFFFFFF;
    gdt.tss_low.limit_low = sizeof(tss);

    gdt_ptr.limit = sizeof(struct gdt) - 1;
    gdt_ptr.base = (uint64_t)&gdt;

    load_gdt((uint64_t)&gdt_ptr);
}