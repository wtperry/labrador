#include "idt.h"
#include "exception.h"
#include <stdio.h>
#include <string.h>

#define IDT_FLAGS_64_CALL       0xC
#define IDT_FLAGS_64_INT        0xE
#define IDT_FLAGS_64_TRAP       0xF
#define IDT_FLAGS_RING0         (0 << 5)
#define IDT_FLAGS_RING1         (1 << 5)
#define IDT_FLAGS_RING2         (2 << 5)
#define IDT_FLAGS_RING3         (3 << 5)
#define IDT_FLAGS_PRESENT       (1 << 7)

#define IDT_FLAGS_INTERRUPT     (IDT_FLAGS_64_INT | IDT_FLAGS_RING0 | IDT_FLAGS_PRESENT)

struct idt_entry idt[MAX_INTERRUPTS];
struct idt_ptr idtr;

extern void load_idt(uint64_t);
//extern void gen_interrupt(uint8_t);

extern uint64_t isr_stub_table[];

void idt_set_gate(int num, uint64_t base, uint16_t sel, uint8_t flags) {
    if (num > MAX_INTERRUPTS) {
        return;
    }

    idt[num].base_low = base & 0xFFFF;
    idt[num].base_middle = (base >> 16) & 0xFFFF;
    idt[num].base_high = (base >> 32) & 0xFFFFFFFF;
    idt[num].sel = sel;
    idt[num].reserved0 = 0;
    idt[num].reserved1 = 0;
    idt[num].flags = flags;
}

// default hander for unhandled system interrupts
void default_handler() {
    printf("Unhandled Exception");

    for(;;);
}

void init_idt() {
    idtr.limit = (sizeof(struct idt_entry) * MAX_INTERRUPTS) - 1;
    idtr.base = (uint64_t)&idt;

    //fill first 32 with exception handlers
    for (size_t i = 0; i < 32; i++) {
        idt_set_gate(i, isr_stub_table[i], 0x08, IDT_FLAGS_INTERRUPT);
    }

    // fill rest with default handlers
    for (int i = 32; i < MAX_INTERRUPTS; i++) {
        idt_set_gate(i, (uint64_t)&default_handler, 0x08, IDT_FLAGS_INTERRUPT);
    }

    load_idt((uint64_t)&idtr);

    init_exceptions();
}