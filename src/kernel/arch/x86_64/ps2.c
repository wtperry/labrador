#include <kernel/arch/ps2.h>

#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include <kernel/arch/apic.h>
#include <kernel/arch/cpu.h>
#include "idt/exception.h"

#define KEYBOARD_IRQ 1

#define PS2_DATA        0x60
#define PS2_COMMAND     0x64
#define PS2_STATUS      0x64

static int ps2_wait_input(void) {
    int timeout = 100000;
    while (--timeout) {
        if (!(inb(PS2_STATUS) & 2)) return 0;
    }
    return 1;
}

static int ps2_wait_output(void) {
    int timeout = 100000;
    while (--timeout) {
        if (!(inb(PS2_STATUS) & 1)) return 0;
    }
    return 1;
}

static void kbd_irq_handler(struct interrupt_data *r) {
	uint8_t scan_code = inb(PS2_DATA);
	apic_eoi();
	printf("%x ", scan_code);
}

void ps2_init(void) {
    // Disable ps2 devices
    outb(PS2_COMMAND, 0xAD);
    outb(PS2_COMMAND, 0xA7);

    // Get rid of data in the output buffer
    while (inb(PS2_STATUS) & 1) {
        inb(PS2_DATA);
    }

    // Set controller config byte
    outb(PS2_COMMAND, 0x20);
    uint8_t config_byte = inb(PS2_DATA);
    config_byte &= ~((1 << 6) | (1 << 1) | (1 << 0));
    outb(PS2_COMMAND, 0x60);
    outb(PS2_DATA, config_byte);

    // Setup Interrupts
    register_exception_handler(&kbd_irq_handler, 32);
    ioapic_set_irq(KEYBOARD_IRQ, 0, 32);

    // Perform self test
    outb(PS2_COMMAND, 0xAA);
    // Wait for response
    while (!(inb(PS2_STATUS) & 1));
    if (inb(PS2_DATA) != 0x55) {
        printf("PS2 self test failed!\n");
        return;
    }

    // Perform interface tests
    outb(PS2_COMMAND, 0xAB);
    while (!(inb(PS2_STATUS) & 1));
    if (inb(PS2_DATA)) {
        printf("Port 1 failed!\n");
    }

    outb(PS2_COMMAND, 0xA9);
    while (!(inb(PS2_STATUS) & 1));
    if (inb(PS2_DATA)) {
        printf("Port 2 failed!\n");
    }

    // Enable Ports
    outb(PS2_COMMAND, 0xAE);
    outb(PS2_COMMAND, 0xA8);

    // Enable Keyboard IRQ
    outb(PS2_COMMAND, 0x20);
    config_byte = inb(PS2_DATA);
    config_byte |= 1;
    outb(PS2_COMMAND, 0x60);
    outb(PS2_DATA, config_byte);
}