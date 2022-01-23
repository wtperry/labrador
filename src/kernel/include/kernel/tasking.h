#pragma once

#include <stddef.h>
#include <stdint.h>

typedef enum state {
    STATE_READY,
    STATE_RUNNING,
    STATE_SLEEPING,
    STATE_BLOCKED
} state_t;

typedef struct thread {
    void* kernel_stack;
    uint64_t cr3;
    state_t state;
} thread_t;

typedef struct sleeper {
    thread_t *thread;
    uint64_t wake_time;
} sleeper_t;

#include <kernel/arch/smp.h>

void tasking_init(void);
void switch_next(void);
thread_t *create_task(void (*func)(void));
extern void switch_task(thread_t *thread);
void sleep_until(uint64_t wake_time);
void sleep(uint64_t sleep_time);
void wakeup_sleepers(void);