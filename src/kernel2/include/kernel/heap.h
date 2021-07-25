#ifndef _KERNEL_HEAP_H
#define _KERNEL_HEAP_H

#include <stddef.h>

void init_heap(void* heap_start, size_t HEAP_SIZE);
void* k_malloc(size_t size);
void k_free(void* ptr);
void dump_heap(void);

#endif