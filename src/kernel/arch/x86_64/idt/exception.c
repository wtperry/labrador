#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include "exception.h"

void (*handlers[32])(struct exception_data*);

void isr_exception_handler(struct exception_data* ex_data) {
    if (handlers[ex_data->int_num]) {
        handlers[ex_data->int_num](ex_data);
        return;
    }

    printf("INTERRUPT: Exception no %x\n", ex_data->int_num);
    printf("rax=%.16x rbx=%.16x rcx=%.16x rdx=%.16x\n", ex_data->rax, ex_data->rbx, ex_data->rcx, ex_data->rdx);
    printf("rsi=%.16x rdi=%.16x rbp=%.16x rsp=%.16x\n", ex_data->rsi, ex_data->rdi, ex_data->rbp, ex_data->user_rsp);
    printf("r8 =%.16x r9 =%.16x r10=%.16x r11=%.16x\n", ex_data->r8, ex_data->r9, ex_data->r10, ex_data->r11);
    printf("r12=%.16x r13=%.16x r14=%.16x r15=%.16x\n", ex_data->r12, ex_data->r13, ex_data->r14, ex_data->r15);
    printf("cs=%.4x ds=%.4x es=%.4x fs=%.4x gs=%.4x ss=%.4x\n", ex_data->cs, ex_data->ds, ex_data->es, ex_data->fs, ex_data->gs, ex_data->ss);
    printf("Halting kernel...\n");
    
    for(;;) {
        asm("hlt");
    }
}

void init_exceptions() {
    for (size_t i = 0; i < 32; i++) {
        handlers[i] = 0;
    }
}

void register_exception_handler(void (*handler)(struct exception_data*), uint8_t ex_num) {
    handlers[ex_num] = handler;
}