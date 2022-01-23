#pragma once

#include <stdint.h>

typedef volatile struct spinlock {
    int lock;
    uint64_t rflags;
} spinlock_t;

//#define spin_init(spinlock) do {spinlock->lock = 0;} while (0);
//#define spin_acquire(spinlock) do {while (__sync_lock_test_and_set(&spinlock, 1)) {asm volatile ("pause");}} while(0);
//#define spin_release(spinlock) do {__sync_lock_release(&spinlock);} while(0);

void spin_init(spinlock_t *spinlock);
void spin_acquire(spinlock_t *spinlock);
void spin_release(spinlock_t *spinlock);