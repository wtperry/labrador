#include "gdt.h"

struct gdt_entry gdt[5];
struct gdt_ptr gp;

extern void load_gdt(uint64_t);

void gdt_set_desc(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags) {
    gdt[num].base_low = base & 0xFFFF;
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high = (base >> 24) & 0xFF;

    gdt[num].limit_low = limit & 0xFFFF;
    gdt[num].limit_high = (limit >> 16) & 0x0F;

    gdt[num].limit_high |= (flags & 0xF0);

    gdt[num].access = access;
}

void init_gdt() {
    gp.limit = (sizeof(struct gdt_entry) * 5) - 1;
    gp.base = (uint64_t)&gdt;

    // null descriptor
    gdt_set_desc(0, 0, 0, 0, 0);
    // kernel code
    gdt_set_desc(1, 0, 0, GDT_PRESENT | GDT_CODE | GDT_RING0, GDT_FLAG_LONG);
    // kernel data
    gdt_set_desc(2, 0, 0, GDT_PRESENT | GDT_DATA | GDT_RING0, GDT_FLAG_LONG);
    // user code
    gdt_set_desc(3, 0, 0, GDT_PRESENT | GDT_CODE | GDT_RING3, GDT_FLAG_LONG);
    // user data
    gdt_set_desc(4, 0, 0, GDT_PRESENT | GDT_DATA | GDT_RING3, GDT_FLAG_LONG);

    load_gdt((uint64_t)&gp);
}