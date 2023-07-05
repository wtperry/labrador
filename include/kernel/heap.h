#pragma once

#include <stddef.h>

void heap_init(void *heap_start, size_t num_pages);
void* kmalloc(size_t size);
void kfree(void* ptr);
void dump_heap(void);