#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <kernel/arch/paging.h>

struct vmm_avl {
    vaddr_t start_addr;
    size_t size;
    bool used;

    size_t height;

    struct vmm_avl* left;
    struct vmm_avl* right;
};

struct vmm_avl* vmm_avl_new(vaddr_t start_addr, size_t size, bool used);
struct vmm_avl* vmm_avl_insert(struct vmm_avl* tree, addr_t start_addr, size_t size, bool used);
struct vmm_avl* vmm_avl_delete(struct vmm_avl* tree, addr_t start_addr);