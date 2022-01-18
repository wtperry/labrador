#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct thread {
    void* kernel_stack;
    uint64_t cr3;
} thread_t;

void tasking_init(void);
void switch_next(void);
thread_t *create_task(void (*func)(void));
extern void switch_task(thread_t *thread);