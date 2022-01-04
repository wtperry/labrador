#pragma once

#include <stddef.h>

void init_heap(void);
void* kmalloc(size_t size);
void kfree(void* ptr);
void dump_heap(void);