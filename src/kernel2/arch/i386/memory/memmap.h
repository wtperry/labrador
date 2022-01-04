#ifndef MEMMAP_H
#define MEMMAP_H

#include <stddef.h>

extern void* _kernel_end;
#define KERNEL_HEAP_START &_kernel_end
#define PAGE_MAP_BASE 0xFFC00000
#define KERNEL_HEAP_SIZE PAGE_MAP_BASE - (size_t) KERNEL_HEAP_START

#endif