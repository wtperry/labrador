#include <kernel/heap.h>
#include <kernel/vmm.h>
#include <kernel/arch/paging.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define INIT_HEAP_PAGES 1

#define HEAP_ALIGN 8

struct mem_header {
    struct mem_header* next;
    struct mem_header* prev;
    bool used;
    size_t size;
};

struct mem_header* heap_start;
struct mem_header* next_free = 0;

struct mem_header* expand_heap(size_t min_size) {
    size_t num_pages = (min_size + 4095) / 4096;
    struct mem_header* new_block = (struct mem_header*) vmm_alloc_region(num_pages);
    new_block->used = false;
    new_block->next = NULL;
    new_block->prev = NULL;
    new_block->size = num_pages * 4096 - sizeof(struct mem_header);
    return new_block;
}

void init_heap() {
    heap_start = (struct mem_header*)vmm_alloc_region(INIT_HEAP_PAGES);
    next_free = heap_start;
    next_free->next = NULL;
    next_free->prev = NULL;
    next_free->used = false;
    next_free->size = INIT_HEAP_PAGES * 4096;
}

void* kmalloc(size_t size) {
    size = (size + HEAP_ALIGN - 1)/HEAP_ALIGN * HEAP_ALIGN;

    if (!next_free) {
        next_free = expand_heap(size);
    }

    struct mem_header* block = next_free;

    while (block) {
        if (!block->used && block->size >= size) {
            if (block->size > size + sizeof(struct mem_header) + HEAP_ALIGN) {
                struct mem_header* new_block = (struct mem_header*)((uintptr_t)block + sizeof(struct mem_header) + size);
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

            if (next_free == block) {
                next_free = block->next;
            }
            
            while (next_free && next_free->used) {
                next_free = next_free->next;
            }

            return (void*)((size_t)block + sizeof(struct mem_header));
        }

        if (!block->next) {
            block->next = expand_heap(size);
            block->next->prev = block;
        }

        block = block->next;
    }

    return NULL;
}

void kfree(void* ptr) {
    // Do nothing when ptr is NULL
    if (!ptr) {
        return;
    }

    struct mem_header* to_free = (struct mem_header*)((size_t)ptr - sizeof(struct mem_header));
    memset(ptr, 0, to_free->size);
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