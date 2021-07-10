#include <kernel/heap.h>
#include <stdbool.h>
#include <stdio.h>

typedef struct mem_header {
    struct mem_header* next;
    struct mem_header* prev;
    bool used;
    size_t size;
} mem_header;

mem_header* start_heap;
mem_header* next_free = 0;

void init_heap(void* heap_start, size_t heap_size) {
    start_heap = heap_start;
    next_free = heap_start;
    next_free->next = 0;
    next_free->prev = 0;
    next_free->used = false;
    next_free->size = heap_size - sizeof(mem_header);
}

void* k_malloc(size_t size) {
    mem_header* block = next_free;
    
    while (block) {
        if (!block->used && block->size >= size) {
            if (block->size > size + sizeof(mem_header)) {
                mem_header* new_block = (mem_header*)((size_t)block + sizeof(mem_header) + size);
                new_block->next = block->next;
                new_block->prev = block;
                new_block->used = false;
                new_block->size = block->size - size - sizeof(mem_header);
                if (block->next) {
                    block->next->prev = new_block;
                }
                block->next = new_block;
                block->size = size;
            }

            block->used = true;

            next_free = block->next;
            
            while (next_free && next_free->used) {
                next_free = next_free->next;
            }

            return (void*)((size_t)block + sizeof(mem_header));
        }

        block = block->next;
    }

    return 0;
}

void k_free(void* ptr) {
    mem_header* to_free = (mem_header*)((size_t)ptr - sizeof(mem_header));
    to_free->used = false;
    if (to_free->prev && !to_free->prev->used) {
        to_free->prev->size += to_free->size + sizeof(mem_header);
        to_free->prev->next = to_free->next;
        if (to_free->next) {
            to_free->next->prev = to_free->prev;
        }

        to_free = to_free->prev;
    }

    if (to_free->next && !to_free->next->used) {
        to_free->size += to_free->next->size + sizeof(mem_header);
        to_free->next = to_free->next->next;
        if (to_free->next) {
            to_free->next->prev = to_free;
        }
    }

    if (!next_free || next_free > to_free) {
        next_free = to_free;
    }
}

void dump_heap() {
    printf("HEAP:\n");
    mem_header* block = start_heap;
    while (block) {
        printf("addr: %x    size: %x    used: %d\n", block, block->size, block->used);
        block = block->next;
    }
}