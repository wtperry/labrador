#pragma once

#include <stdint.h>

void apic_init(void);
void apic_eoi(void);
void ioapic_set_irq(uint8_t irq, uint64_t apic_id, uint8_t int_num);
void ioapic_mask_irq(uint8_t irq);
void apic_timer_init(uint64_t freq);