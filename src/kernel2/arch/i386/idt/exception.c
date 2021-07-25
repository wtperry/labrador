#include <kernel/hal.h>
#include <stdint.h>
#include <stdio.h>

struct exception_data {
    uint32_t gs, fs, es, ds;
    uint32_t edi, esi, ebp, esp , ebx, edx, ecx, eax;
    uint32_t int_num, error_code;
    uint32_t eip, cs, eflags, user_esp, ss;
};

void isr_exception_handler(struct exception_data* ex_data) {
    disable_interrupts();

    printf("INTERRUPT: Exception no %d\n", ex_data->int_num);
    printf("Halting kernel...");
    
    for(;;) {
        asm ("hlt");
    }
}