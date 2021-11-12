#pragma once

#include <stdint.h>

#define PAGE_FAULT 14

struct exception_data {
    uint64_t cr2;
    uint64_t gs, fs, es, ds;
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rax, rcx, rdx, rbx, rsp, rbp, rsi, rdi;
    uint64_t int_num, error_code;
    uint64_t rip, cs, rflags, user_rsp, ss;
};

void init_exceptions(void);
void register_exception_handler(void (*)(struct exception_data*), uint8_t);