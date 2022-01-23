#include <kernel/arch/apic.h>

#include <stdint.h>

#include <kernel/tasking.h>

#include <kernel/arch/cpu.h>
#include <kernel/arch/paging.h>
#include <kernel/arch/time.h>
#include "idt/idt.h"
#include "idt/exception.h"

#define APIC_TMR_DIV_REG        0x3E0
#define APIC_TMR_DIV_16         0x3
#define APIC_TMR_INIT_COUNT_REG 0x380
#define APIC_TMR_CURR_COUNT_REG 0x390
#define APIC_TMR_MODE_PERIODIC  (1 << 17)
#define APIC_LVT_TMR_REG        0x320
#define APIC_LVT_MASK           (1 << 16)

#define APIC_TMR_INT            33

#define PIC_OCW2_L1             (1 << 0)    // Level 1 interrupt
#define PIC_OCW2_L2             (1 << 1)    // Level 2 interrupt
#define PIC_OCW2_L3             (1 << 2)    // Level 3 interrupt
#define PIC_OCW2_EOI            (1 << 5)    // End of Interrupt command
#define PIC_OCW2_SEL            (1 << 6)    // selection command
#define PIC_OCW2_ROT            (1 << 7)    // rotation command

#define PIC_OCW3_IRR            0x0a
#define PIC_OCW3_ISR            0x0b

#define PIC_ICW1_ICW4           (1 << 0)    // set if PIC should expect ICW4
#define PIC_ICW1_SNGL           (1 << 1)    // set if only one PIC
#define PIC_ICW1_ADI            (1 << 2)    // set if CALL address interval is 4, otherwise 8 (default 0)
#define PIC_ICW1_MODE           (1 << 3)    // set if level triggered mode, else edge triggered mode
#define PIC_ICW1_INIT           (1 << 4)    // set if PIC is to be initiallized

#define PIC_ICW4_UPM            (1 << 0)    // set if 80x86 mode, otherwise MCS-80/86 mode
#define PIC_ICW4_AEOI           (1 << 1)    // automatic end of interrupt
#define PIC_ICW4_MS             (1 << 2)    // only used if BUF is set, set if select buffer master, otherwise slave
#define PIC_ICW4_BUF            (1 << 3)    // set if controller operates in buffered mode
#define PIC_ICW4_SFNM           (1 << 4)    // Specially Fully Nested Mode

#define PIC1_REG_COMMAND        0x20
#define PIC1_REG_DATA           0x21

#define PIC2_REG_COMMAND        0xA0
#define PIC2_REG_DATA           0xA1

#define IRQ0_INT                0x20
#define IRQ8_INT                0x28

void disable_pic(void) {
    // ICW1 - Initialization
    outb(PIC1_REG_COMMAND, PIC_ICW1_ICW4 | PIC_ICW1_INIT);
    outb(PIC2_REG_COMMAND, PIC_ICW1_ICW4 | PIC_ICW1_INIT);

    // ICW2 - IRQ offsets
    outb(PIC1_REG_DATA, IRQ0_INT);
    outb(PIC2_REG_DATA, IRQ8_INT);

    // ICW3 - IRW Line 2
    outb(PIC1_REG_DATA, 4);
    outb(PIC2_REG_DATA, 2);

    // ICW4 - Mode
    outb(PIC1_REG_DATA, 1);
    outb(PIC2_REG_DATA, 1);

    // OCW1 - interrupt masks
    outb(PIC1_REG_DATA, 0xFF);
    outb(PIC2_REG_DATA, 0xFF);
}

// FIXME: read from ACPI tables
const uintptr_t ioapic_base = PHYS_TO_VIRT(0xFEC00000);
const uintptr_t apic_base = PHYS_TO_VIRT(0xFEE00000);

uint32_t apic_read(uint32_t offset) {
    return *(uint32_t*)(apic_base + offset);
}

void apic_write(uint32_t offset, uint32_t value) {
    *(uint32_t*)(apic_base + offset) = value;
}

uint32_t ioapic_read(uint32_t index) {
    *(uint32_t*)ioapic_base = index;
    return *(uint32_t*)(ioapic_base + 0x10);
}

void ioapic_write(uint32_t index, uint32_t value) {
    *(uint32_t*)ioapic_base = index;
    *(uint32_t*)(ioapic_base + 0x10) = value;
}

void ioapic_set_irq(uint8_t irq, uint64_t apic_id, uint8_t int_num) {
    const uint32_t low_index = 0x10 + irq*2;
    const uint32_t high_index = 0x10 + irq*2 + 1;

    uint32_t high = ioapic_read(high_index);
    // set APIC ID
    high &= ~0xff000000;
    high |= apic_id << 24;
    ioapic_write(high_index, high);

    uint32_t low = ioapic_read(low_index);

    // unmask the IRQ
    low &= ~(1<<16);

    // set to physical delivery mode
    low &= ~(1<<11);

    // set to fixed delivery mode
    low &= ~0x700;

    // set delivery vector
    low &= ~0xff;
    low |= int_num;

    ioapic_write(low_index, low);
}

void ioapic_mask_irq(uint8_t irq) {
    const uint32_t low_index = 0x10 + irq*2;
    uint32_t low = ioapic_read(low_index);
    low |= (1 << 16);
    ioapic_write(low_index, low);
}

void apic_eoi(void) {
    apic_write(0xB0, 0);
}

void apic_init(void) {
    disable_pic();

    apic_write(0xF0, 0x1FF);
}

void apic_timer_isr(struct interrupt_data *r) {
    update_clock();
    apic_eoi();

    switch_next();
}

void apic_timer_init(uint64_t freq) {
    uint64_t tsc_ticks = tsc_freq / 100; // Time for 10 ms

    uint64_t tsc_end = read_tsc() + tsc_ticks;
    apic_write(APIC_TMR_DIV_REG, APIC_TMR_DIV_16);
    apic_write(APIC_TMR_INIT_COUNT_REG, 0xFFFFFFFF);

    while (read_tsc() < tsc_end);

    uint64_t apic_freq = (0xFFFFFFFF - apic_read(APIC_TMR_CURR_COUNT_REG)) * 100;

    printf("APIC Timer Frequency: %lu hz\n", apic_freq);

    register_exception_handler(&apic_timer_isr, APIC_TMR_INT);
    apic_write(APIC_LVT_TMR_REG, APIC_TMR_MODE_PERIODIC | APIC_TMR_INT);
    apic_write(APIC_TMR_INIT_COUNT_REG, apic_freq / freq);
}