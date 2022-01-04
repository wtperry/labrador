#include "gdt.h"

struct gdt_entry gdt[3];
struct gdt_ptr gp;

extern void load_gdt(uint32_t);

void gdt_set_desc(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt[num].base_low = base & 0xFFFF;
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high = (base >> 24) & 0xFF;

    gdt[num].limit_low = limit & 0xFFFF;
    gdt[num].granularity = (limit >> 16) & 0x0F;

    gdt[num].granularity |= (gran & 0xF0);

    gdt[num].access = access;
}

void init_gdt() {
    gp.limit = (sizeof(struct gdt_entry) * 5) - 1;
    gp.base = (uint32_t)&gdt;

    // null descriptor
    gdt_set_desc(0, 0, 0, 0, 0);
    // kernel code
    gdt_set_desc(1, 0, 0xFFFFFFFF, 0x9a, 0xCF);
    // kernel data
    gdt_set_desc(2, 0, 0xFFFFFFFF, 0x92, 0xCF);
    // user code
    //gdt_set_desc(3, 0, 0xFFFFFFFF, 0xFa, 0xCF);
    // user data
    //gdt_set_desc(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);

    load_gdt((uint32_t)&gp);
}