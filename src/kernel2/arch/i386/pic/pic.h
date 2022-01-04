#ifndef IDT_H
#define IDT_H

#define IRQ_NUM 16

// PIC 1 IRQs
#define PIC_IRQ_TIMER       0
#define PIC_IRQ_KEYBOARD    1
#define PIC_IRQ_SLAVEPIC    2
#define PIC_IRQ_SERIAL2     3
#define PIC_IRQ_SERIAL1     4
#define PIC_IRQ_PARALLEL2   5
#define PIC_IRQ_DISKETTE    6
#define PIC_IRQ_PARALLEL1   7

// PIC 2 IRQs
#define PIC_IRQ_CMOSTIMER   8
#define PIC_IRQ_CGARETRACE  9
#define PIC_IRQ_MOUSE       12
#define PIC_IRQ_FPU         13
#define PIC_IRQ_HDC1        14
#define PIC_IRO_HDC2        15

void pic_set_irq_mask(uint8_t irq);
void pic_clear_irq_mask(uint8_t irq);

int pic_check_irq_in_service(uint8_t irq);

void pic_notify_interrupt_complete(uint8_t irq);

void pic_send_data(uint8_t data, int pic_num);
void pic_send_command(uint8_t cmd, int pic_num);
uint8_t pic_read_data(int pic_num);

void init_pic(void);

#endif