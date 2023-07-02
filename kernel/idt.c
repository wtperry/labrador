#include <kernel/idt.h>

#include <kernel/gdt.h>
#include <kernel/string.h>
#include <kernel/log.h>

#include <stdint.h>

#define IDT_FLAGS_64_CALL       0xC
#define IDT_FLAGS_64_INT        0xE
#define IDT_FLAGS_64_TRAP       0xF
#define IDT_FLAGS_RING0         (0 << 5)
#define IDT_FLAGS_RING1         (1 << 5)
#define IDT_FLAGS_RING2         (2 << 5)
#define IDT_FLAGS_RING3         (3 << 5)
#define IDT_FLAGS_PRESENT       (1 << 7)

#define IDT_FLAGS_INTERRUPT     (IDT_FLAGS_64_INT | IDT_FLAGS_RING0 | IDT_FLAGS_PRESENT)

#define NUM_INTERRUPTS 256

struct idt_entry {
    uint16_t base_low;
    uint16_t sel;
    uint8_t reserved0;
    uint8_t flags;
    uint16_t base_middle;
    uint32_t base_high;
    uint32_t reserved1;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

struct idt_entry idt[NUM_INTERRUPTS];
struct idt_ptr idtr;

extern uint64_t isr_stub_table[];

void (*handlers[NUM_INTERRUPTS])(struct interrupt_data*);

static void idt_set_gate(int num, uint64_t base, uint16_t sel, uint8_t flags) {
    if (num > NUM_INTERRUPTS) {
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

void default_int_handler(struct interrupt_data* ex_data) {
    if (handlers[ex_data->int_num]) {
        handlers[ex_data->int_num](ex_data);
        return;
    }

    log_printf(LOG_FATAL, "UNHANDLED INTERRUPT: Int no %x, error code: %lu", ex_data->int_num, ex_data->error_code);
    log_printf(LOG_FATAL, "rax=%.16lx rbx=%.16lx rcx=%.16lx rdx=%.16lx", ex_data->rax, ex_data->rbx, ex_data->rcx, ex_data->rdx);
    log_printf(LOG_FATAL, "rsi=%.16lx rdi=%.16lx rbp=%.16lx rsp=%.16lx", ex_data->rsi, ex_data->rdi, ex_data->rbp, ex_data->user_rsp);
    log_printf(LOG_FATAL, "r8 =%.16lx r9 =%.16lx r10=%.16lx r11=%.16lx", ex_data->r8, ex_data->r9, ex_data->r10, ex_data->r11);
    log_printf(LOG_FATAL, "r12=%.16lx r13=%.16lx r14=%.16lx r15=%.16lx", ex_data->r12, ex_data->r13, ex_data->r14, ex_data->r15);
    //log_printf(LOG_FATAL, "cs=%.4x ds=%.4x es=%.4x fs=%.4x gs=%.4x ss=%.4x\n", ex_data->cs, ex_data->ds, ex_data->es, ex_data->fs, ex_data->gs, ex_data->ss);
    log_printf(LOG_FATAL, "Halting kernel...");
    
    for(;;) {
        asm volatile ("hlt");
    }
}

void idt_init() {
    idtr.limit = (sizeof(struct idt_entry) * NUM_INTERRUPTS) - 1;
    idtr.base = (uint64_t)&idt;

    //fill idt with isr stubs
    for (size_t i = 0; i < NUM_INTERRUPTS; i++) {
        idt_set_gate(i, isr_stub_table[i], GDT_KERNEL_CODE_DESC, IDT_FLAGS_INTERRUPT);
    }

    asm volatile ("lidt %0" : : "m"(idtr));

    for (size_t i = 0; i < NUM_INTERRUPTS; i++) {
        handlers[i] = 0;
    }
}

void idt_register_handler(void (*handler)(struct interrupt_data*), uint8_t int_num) {
    handlers[int_num] = handler;
}