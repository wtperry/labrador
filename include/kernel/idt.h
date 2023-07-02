#pragma once

#include <stdint.h>

struct interrupt_data {
    // uint64_t gs, fs, es, ds;
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rax, rcx, rdx, rbx, rsp, rbp, rsi, rdi;
    uint64_t int_num, error_code;
    uint64_t rip, cs, rflags, user_rsp, ss;
};

void idt_init();
void idt_register_handler(void (*handler)(struct interrupt_data*), uint8_t int_num);