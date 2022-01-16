#include <kernel/arch/cpu.h>

#include <stdint.h>

uint64_t read_msr(uint32_t msr) {
    uint32_t low, high;
    asm volatile ("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));

    return ((uint64_t)high << 32) + low;
}

void load_msr(uint32_t msr, uint64_t value) {
    uint32_t high = (value >> 32) & 0xFFFFFFFF;
    uint32_t low = value & 0xFFFFFFFF;
    asm volatile ("wrmsr" : : "a"(low), "d" (high), "c"(msr));
}

void outb(uint16_t port, uint8_t data) {
    asm volatile ("outb %0, %1" : : "a"(data), "Nd"(port));
}

uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd" (port));
    return ret;
}

void enable_interrupts(void) {
    asm volatile ("sti");
}

void disable_interrupts(void) {
    asm volatile ("cli");
}

void cpuid(int code, uint32_t *a, uint32_t *b, uint32_t *c, uint32_t *d) {
    asm volatile ("cpuid" : "=a"(*a), "=b"(*b), "=c"(*c), "=d"(*d) : "a"(code));
}