#pragma once

#include <stdint.h>

uint64_t read_msr(uint32_t msr);
void load_msr(uint32_t msr, uint64_t value);

void outb(uint16_t port, uint8_t data);
uint8_t inb(uint16_t port);

void enable_interrupts(void);
void disable_interrupts(void);

void cpuid(int code, uint32_t *a, uint32_t *b, uint32_t *c, uint32_t *d);