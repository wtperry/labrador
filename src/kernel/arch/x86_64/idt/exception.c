#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include "exception.h"

void (*handlers[34])(struct interrupt_data*);

void isr_handler(struct interrupt_data* ex_data) {
    if (handlers[ex_data->int_num]) {
        handlers[ex_data->int_num](ex_data);
        return;
    }

    printf("INTERRUPT: Exception no %x, error code: %lu\n", ex_data->int_num, ex_data->error_code);
    printf("rax=%.16lx rbx=%.16lx rcx=%.16lx rdx=%.16lx\n", ex_data->rax, ex_data->rbx, ex_data->rcx, ex_data->rdx);
    printf("rsi=%.16lx rdi=%.16lx rbp=%.16lx rsp=%.16lx\n", ex_data->rsi, ex_data->rdi, ex_data->rbp, ex_data->user_rsp);
    printf("r8 =%.16lx r9 =%.16lx r10=%.16lx r11=%.16lx\n", ex_data->r8, ex_data->r9, ex_data->r10, ex_data->r11);
    printf("r12=%.16lx r13=%.16lx r14=%.16lx r15=%.16lx\n", ex_data->r12, ex_data->r13, ex_data->r14, ex_data->r15);
    //printf("cs=%.4x ds=%.4x es=%.4x fs=%.4x gs=%.4x ss=%.4x\n", ex_data->cs, ex_data->ds, ex_data->es, ex_data->fs, ex_data->gs, ex_data->ss);
    printf("Halting kernel...\n");
    
    for(;;) {
        asm("hlt");
    }
}

void init_exceptions() {
    for (size_t i = 0; i < 34; i++) {
        handlers[i] = 0;
    }
}

void register_exception_handler(void (*handler)(struct interrupt_data*), uint8_t int_num) {
    handlers[int_num] = handler;
}