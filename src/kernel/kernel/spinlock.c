#include <kernel/spinlock.h>

void spin_init(spinlock_t *spinlock) {
    spinlock->lock = 0;
    spinlock->rflags = 0;
}

void spin_acquire(spinlock_t *spinlock) {
    uint64_t old_rflags = 0;
    asm volatile ("pushfq; \
                   popq %0; \
                   cli;"
                   : "=r"(old_rflags));
    
    while (__sync_lock_test_and_set(&spinlock->lock, 1)) {
        asm volatile ("pause");
    }
    spinlock->rflags = old_rflags;
}
void spin_release(spinlock_t *spinlock) {
    __sync_lock_release(&spinlock->lock);
    asm volatile ("pushq %0; \
                   popfq;"
                  : : "r"(spinlock->rflags));
}