#pragma once

typedef volatile int spinlock_t;

#define spin_init(spinlock) do {spinlock = 0;} while (0);
#define spin_acquire(spinlock) do {while (__sync_lock_test_and_set(&spinlock, 1));} while(0);
#define spin_release(spinlock) do {__sync_lock_release(&spinlock);} while(0);