#include <kernel/arch/pit.h>

#include <stdint.h>
#include <libk/stdio.h>

#include <kernel/arch/apic.h>
#include <kernel/arch/cpu.h>
#include "idt/exception.h"

#include <kernel/tasking.h>

#define PIT_DATA_0      0x40
#define PIT_DATA_1      0x41
#define PIT_DATA_2      0x42
#define PIT_COMMAND     0x43

#define PIT_BCD         (1 << 0)
#define PIT_MODE_0      (0 << 1)
#define PIT_MODE_1      (1 << 1)
#define PIT_MODE_2      (2 << 1)
#define PIT_MODE_3      (3 << 1)
#define PIT_MODE_4      (4 << 1)
#define PIT_MODE_5      (5 << 1)
#define PIT_LSB         (1 << 4)
#define PIT_MSB         (2 << 4)
#define PIT_BOTH        (3 << 4)
#define PIT_COUNTER_0   (0 << 6)
#define PIT_COUNTER_1   (1 << 6)
#define PIT_COUNTER_2   (2 << 6)

// FIXME: parse acpi for this
#define PIT_IRQ 2

#define PIT_INTERRUPT 33

#define PIT_FREQ 1193182

uint64_t ticks;

static void pit_isr(struct interrupt_data *r) {
    ticks++;
    apic_eoi();
}

uint64_t pit_get_ticks(void) {
    return ticks;
}

void pit_init(int freq) {
    ticks = 0;

    register_exception_handler(&pit_isr, PIT_INTERRUPT);
    ioapic_set_irq(PIT_IRQ, 0, PIT_INTERRUPT);

    printf("%x\n", PIT_FREQ/freq);

    outb(PIT_COMMAND, PIT_COUNTER_0 | PIT_BOTH | PIT_MODE_2);
    outb(PIT_DATA_0, (PIT_FREQ/freq) & 0xFF);
    outb(PIT_DATA_0, ((PIT_FREQ/freq) >> 8) & 0xFF);
}

void pit_stop(void) {
    ioapic_mask_irq(PIT_IRQ);
}