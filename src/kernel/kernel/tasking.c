#include <kernel/tasking.h>

#include <stdio.h>

#include <kernel/heap.h>
#include <kernel/vmm.h>
#include <kernel/ds/list.h>
#include <kernel/spinlock.h>

#include <kernel/dev/serial.h>

#include <kernel/arch/time.h>

list_t *ready_list = 0;
list_t *sleep_list = 0;

thread_t *idle_thread = 0;

spinlock_t scheduler_lock;
spinlock_t sleep_lock;

extern void task_start(void);

static void _idle(void) {
    while(1) {
        asm volatile ("hlt");
    }
}

void *init_task_stack(void* stack, void (*func)(void)) {
    uintptr_t *sp = (uintptr_t *)stack;
    *--sp = (uintptr_t)func;
    *--sp = (uintptr_t)&task_start;
    for (size_t i = 0; i < 6; i++) {
        *--sp = 0;
    }

    return (void *) sp;
}

/**
 * @brief Switches to the next thread in the run queue.
 */
void switch_next(void) {
    if (!ready_list) {
        return;
    }

    spin_acquire(&scheduler_lock);
    write_serial("Entering Scheduler!\r\n");

    if (!ready_list->head) {
        if (this_core->current_thread->state == STATE_RUNNING) {
            write_serial("Exiting Scheduler!\r\n");
            spin_release(&scheduler_lock);
            return;
        } else {
            write_serial("Exiting Scheduler!\r\n");
            spin_release(&scheduler_lock);
            switch_task(idle_thread);
        }
    }

    if (this_core->current_thread->state == STATE_RUNNING && this_core->current_thread != idle_thread) {
        this_core->current_thread->state = STATE_READY;
        list_append(ready_list, this_core->current_thread);
    }

    thread_t *next_thread = list_pop_front(ready_list);
    next_thread->state = STATE_RUNNING;
    write_serial("Exiting Scheduler!\r\n");
    spin_release(&scheduler_lock);
    switch_task(next_thread);

    __builtin_unreachable();
}

thread_t *create_task(void (*func)(void)) {
    thread_t *thread = kmalloc(sizeof(*thread));
    thread->kernel_stack = (void *)(vmm_alloc_region(2) + 4096 * 2);

    asm volatile ("mov %%cr3, %0" : "=r"(thread->cr3));

    thread->kernel_stack = init_task_stack(thread->kernel_stack, func);
    thread->state = STATE_READY;

    spin_acquire(&scheduler_lock);
    list_append(ready_list, thread);
    spin_release(&scheduler_lock);

    return thread;
}

void tasking_init(void) {
    ready_list = list_create();
    spin_init(&scheduler_lock);

    sleep_list = list_create();
    spin_init(&sleep_lock);

    idle_thread = kmalloc(sizeof(*idle_thread));
    idle_thread->state = STATE_RUNNING;
    asm volatile ("mov %%cr3, %0" : "=r"(idle_thread->cr3));
    idle_thread->kernel_stack = init_task_stack(vmm_alloc_region(1) + 4096, &_idle);

    this_core->current_thread = kmalloc(sizeof(*this_core->current_thread));
    this_core->current_thread->kernel_stack = 0;
    this_core->current_thread->state = STATE_RUNNING;

    asm volatile ("mov %%cr3, %0" : "=r"(this_core->current_thread->cr3));
}

/**
 * @brief Sleep until a given time.
 * 
 * @param wake_time The time to wakeup in microseconds
 */
void sleep_until(uint64_t wake_time) {
    spin_acquire(&sleep_lock);
    write_serial("Entering sleep_until!\r\n");

    list_node_t *curr_node = sleep_list->head;
    
    while (curr_node) {
        sleeper_t *curr_sleeper = (sleeper_t *)curr_node->value;
        if (curr_sleeper->wake_time > wake_time) {
            break;
        }

        curr_node = curr_node->next;
    }

    sleeper_t *sleeper = kmalloc(sizeof(*sleeper));
    sleeper->thread = this_core->current_thread;
    sleeper->wake_time = wake_time;
    list_insert_before(sleep_list, curr_node, sleeper);

    this_core->current_thread->state = STATE_SLEEPING;

    write_serial("Exiting sleep_until!\r\n");
    spin_release(&sleep_lock);

    switch_next();
}

/**
 * @brief Sleep for a given amount of time.
 * 
 * @param sleep_time Time to sleep in microseconds
 */
void sleep(uint64_t sleep_time) {
    uint64_t curr_time = get_time_from_boot();
    sleep_until(curr_time + sleep_time);
}

/**
 * @brief Wakes up sleeping threads if their wake time has passed.
 */
void wakeup_sleepers(void) {
    if (!sleep_list) {
        /* Scheduler hasn't been initiallized yet! */
        return;
    }

    spin_acquire(&sleep_lock);
    write_serial("Entering wakeup!\r\n");

    uint64_t curr_time = get_time_from_boot();

    list_node_t *curr_node = sleep_list->head;
    
    while (curr_node) {
        sleeper_t *curr_sleeper = (sleeper_t *)curr_node->value;
        if (curr_sleeper->wake_time > curr_time) {
            break;
        }

        list_remove(sleep_list, curr_node);

        spin_acquire(&scheduler_lock);
        curr_sleeper->thread->state = STATE_RUNNING;
        list_append(ready_list, curr_sleeper->thread);
        spin_release(&scheduler_lock);

        kfree(curr_sleeper);

        curr_node = sleep_list->head;
    }

    write_serial("Exiting wakeup!\r\n");
    spin_release(&sleep_lock);
}