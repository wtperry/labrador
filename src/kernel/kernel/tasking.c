#include <kernel/tasking.h>

#include <stdio.h>

#include <kernel/heap.h>
#include <kernel/vmm.h>
#include <kernel/ds/list.h>
#include <kernel/spinlock.h>

thread_t *current_thread;
list_t *ready_list;

spinlock_t scheduler_lock;

extern void task_start(void);

void* init_task_stack(void* stack, void (*func)(void)) {
    uintptr_t *sp = (uintptr_t *)stack;
    *--sp = (uintptr_t)func;
    *--sp = (uintptr_t)&task_start;
    for (size_t i = 0; i < 6; i++) {
        *--sp = 0;
    }

    return (void *) sp;
}

void switch_next(void) {
    spin_acquire(scheduler_lock);

    if (!ready_list->head) {
        return;
    }

    list_append(ready_list, current_thread);

    thread_t *next_thread = list_pop_front(ready_list);
    spin_release(scheduler_lock);
    switch_task(next_thread);

    __builtin_unreachable();
}

thread_t *create_task(void (*func)(void)) {
    thread_t *thread = kmalloc(sizeof(*thread));
    thread->kernel_stack = (void *)(vmm_alloc_region(2) + 4096 * 2);

    asm volatile ("mov %%cr3, %0" : "=r"(thread->cr3));

    thread->kernel_stack = init_task_stack(thread->kernel_stack, func);

    spin_acquire(scheduler_lock);
    list_append(ready_list, thread);
    spin_release(scheduler_lock);

    return thread;
}

void tasking_init(void) {
    ready_list = list_create();
    spin_init(scheduler_lock);

    current_thread = kmalloc(sizeof(*current_thread));
    current_thread->kernel_stack = 0;

    asm volatile ("mov %%cr3, %0" : "=r"(current_thread->cr3));
}