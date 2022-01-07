#include <stdlib.h>

#if defined(__is_libk)
    #include <kernel/heap.h>
#endif

void* malloc(size_t size) {
#if defined(__is_libk)
    return kmalloc(size);
#else
    return NULL;
#endif
}

void free(void* ptr) {
#if defined(__is_libk)
    kfree(ptr);
#else
    
#endif
}