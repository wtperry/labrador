#include <kernel/heap.h>
#include <kernel/vmm.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>

struct mem_header {
    struct mem_header* next;
    struct mem_header* prev;
    bool used;
    size_t size;
};

struct mem_header* heap_start;
struct mem_header* next_free = 0;

void init_heap() {
    heap_start = kbrk(0);
    next_free = heap_start;
    next_free->next = 0;
    next_free->prev = 0;
    next_free->used = false;
    next_free->size = (uint64_t)kbrk(1) - (uint64_t)heap_start;
}

void* kmalloc(size_t size) {
    struct mem_header* block = next_free;

    while (block) {
        if (!block->used && block->size >= size) {
            if (block->size > size + sizeof(struct mem_header)) {
                struct mem_header* new_block = (struct mem_header*)((size_t)block + sizeof(struct mem_header) + size);
                new_block->next = block->next;
                new_block->prev = block;
                new_block->used = false;
                new_block->size = block->size - size - sizeof(struct mem_header);
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

            return (void*)((size_t)block + sizeof(struct mem_header));
        }

        block = block->next;
    }

    return NULL;
}

void kfree(void* ptr) {
    struct mem_header* to_free = (struct mem_header*)((size_t)ptr - sizeof(struct mem_header));
    to_free->used = false;
    if (to_free->prev && !to_free->prev->used) {
        to_free->prev->size += to_free->size + sizeof(struct mem_header);
        to_free->prev->next = to_free->next;
        if (to_free->next) {
            to_free->next->prev = to_free->prev;
        }

        to_free = to_free->prev;
    }

    if (to_free->next && !to_free->next->used) {
        to_free->size += to_free->next->size + sizeof(struct mem_header);
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
    struct mem_header* block = heap_start;
    while (block) {
        printf("addr: %.16lx    size: %lx    used: %d\n", block, block->size, block->used);
        block = block->next;
    }
}