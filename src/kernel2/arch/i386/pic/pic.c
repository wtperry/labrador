#include <stdint.h>
#include "pic.h"
#include "../drivers/ports/ports.h"

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

uint16_t _pic_get_isr() {
    outb(PIC1_REG_COMMAND, PIC_OCW3_ISR);
    outb(PIC2_REG_COMMAND, PIC_OCW3_ISR);
    return (inb(PIC2_REG_COMMAND) << 8) | inb(PIC1_REG_COMMAND);
}

void pic_set_irq_mask(uint8_t irq) {
    int pic_num;
    uint8_t imr;

    if (irq < 8) {
        pic_num = 0;
    } else {
        pic_num = 1;
        irq -= 8;
    }

    imr = pic_read_data(pic_num) | (1 << irq);
    pic_send_data(pic_num, imr);
}

void pic_clear_irq_mask(uint8_t irq) {
    int pic_num;
    uint8_t imr;

    if (irq < 8) {
        pic_num = 0;
    } else {
        pic_num = 1;
        irq -= 8;
    }

    imr = pic_read_data(pic_num) & ~(1 << irq);
    pic_send_data(pic_num, imr);
}

int pic_check_irq_in_service(uint8_t irq) {
    return _pic_get_isr() & (1 << irq);
}

void pic_notify_interrupt_complete(uint8_t irq) {
    // Check for valid IRQ
    if (irq >= IRQ_NUM)
        return;

    //Check if IRQ is handled by slave PIC
    if (irq >= 8) {
        pic_send_command(PIC_OCW2_EOI, 1);
    }

    //Always send to master
    pic_send_command(PIC_OCW2_EOI, 0);
}

void pic_send_data(uint8_t data, int pic_num) {
    if (pic_num > 1)
        return;
    
    uint16_t port = (pic_num == 1) ? PIC2_REG_DATA : PIC1_REG_DATA;
    outb(port, data);
}

uint8_t pic_read_data(int pic_num) {
    if (pic_num > 1)
        return 0;
    
    uint16_t port = (pic_num == 1) ? PIC2_REG_DATA : PIC1_REG_DATA;
    return inb(port);
}

void pic_send_command(uint8_t cmd, int pic_num) {
    if (pic_num > 1)
        return;
    
    uint16_t port = (pic_num == 1) ? PIC2_REG_COMMAND : PIC1_REG_COMMAND;
    outb(port, cmd);
}

void init_pic() {
    uint8_t icw = 0;

    //ICW1
    icw = PIC_ICW1_ICW4 | PIC_ICW1_INIT;
    pic_send_command(icw, 0);
    pic_send_command(icw, 1);

    //ICW2
    pic_send_data(IRQ0_INT, 0);
    pic_send_data(IRQ8_INT, 1);

    //ICW3
    // sets both PICs to IR2
    pic_send_data((1 << 2), 0);
    pic_send_data(0x2, 1);

    //ICW4
    icw = 0;
    icw = PIC_ICW4_UPM;
    pic_send_data(icw, 0);
    pic_send_data(icw, 1);
}