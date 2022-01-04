#include "idt.h"
#include <stdio.h>
#include <string.h>

#define IDT_FLAGS_32_TASK       0x5
#define IDT_FLAGS_32_INT        0xE
#define IDT_FLAGS_32_TRAP       0xF
#define IDT_FLAGS_RING0         (0 << 5)
#define IDT_FLAGS_RING1         (1 << 5)
#define IDT_FLAGS_RING2         (2 << 5)
#define IDT_FLAGS_RING3         (3 << 5)
#define IDT_FLAGS_PRESENT       (1 << 7)

#define IDT_FLAGS_INTERRUPT     (IDT_FLAGS_32_INT | IDT_FLAGS_RING0 | IDT_FLAGS_PRESENT)

struct idt_entry idt[I386_MAX_INTERRUPTS];
struct idt_ptr idtr;

extern void load_idt(uint32_t);
extern void gen_interrupt(uint8_t);

extern uint32_t isr_stub_table[];

void idt_set_gate(int num, uint32_t base, uint16_t sel, uint8_t flags) {
    if (num > I386_MAX_INTERRUPTS) {
        return;
    }

    idt[num].base_low = base & 0xFFFF;
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].sel = sel;
    idt[num].reserved = 0;
    idt[num].flags = flags;
}

// default hander for unhandled system interrupts
void i386_default_handler() {
    printf("Unhandled Exception");

    for(;;);
}

void init_idt() {
    idtr.limit = (sizeof(struct idt_entry) * I386_MAX_INTERRUPTS) - 1;
    idtr.base = (uint32_t)&idt;

    //fill first 32 with exception handlers
    for (size_t i = 0; i < 32; i++) {
        idt_set_gate(i, isr_stub_table[i], 0x08, IDT_FLAGS_INTERRUPT);
    }

    // fill rest with default handlers
    for (int i = 32; i < I386_MAX_INTERRUPTS; i++) {
        idt_set_gate(i, (uint32_t)&i386_default_handler, 0x8, IDT_FLAGS_INTERRUPT);
    }

    load_idt((uint32_t)&idtr);
}