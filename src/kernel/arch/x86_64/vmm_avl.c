#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <kernel/heap.h>

#include "vmm_avl.h"

size_t height(struct vmm_avl* tree) {
    if (tree == NULL)
        return 0;
    
    return tree->height;
}

size_t max(size_t a, size_t b) {
    if (a > b)
        return a;
    
    return b;
}

int get_balance(struct vmm_avl* tree) {
    if (tree == NULL)
        return 0;

    return height(tree->left) - height(tree->right);
}

struct vmm_avl* get_min_node(struct vmm_avl* tree) {
    struct vmm_avl* current = tree;

    while (current->left != NULL) {
        current = current->left;
    }

    return current;
}

struct vmm_avl* rotate_right(struct vmm_avl* tree) {
    struct vmm_avl* new_tree = tree->left;
    tree->left = new_tree->right;
    new_tree->right = tree;

    tree->height = 1 + max(height(tree->left), height(tree->right));
    new_tree->height = 1 + max(height(new_tree->left), height(new_tree->right));

    return new_tree;
}

struct vmm_avl* rotate_left(struct vmm_avl* tree) {
    struct vmm_avl* new_tree = tree->right;
    tree->right = new_tree->left;
    new_tree->left = tree;

    tree->height = 1 + max(height(tree->left), height(tree->right));
    new_tree->height = 1 + max(height(new_tree->left), height(new_tree->right));

    return new_tree;
}

struct vmm_avl* vmm_avl_new(vaddr_t start_addr, size_t size, bool used) {
    struct vmm_avl* tree = kmalloc(sizeof(struct vmm_avl));

    tree->start_addr = start_addr;
    tree->size = size;
    tree->used = used;

    tree->height = 1;

    tree->left = NULL;
    tree->right = NULL;

    return tree;
}

struct vmm_avl* vmm_avl_insert(struct vmm_avl* tree, vaddr_t start_addr, size_t size, bool used) {
    if (tree == NULL) {
        return vmm_avl_new(start_addr, size, used);
    }

    if (tree->start_addr > start_addr) {
        tree->left = vmm_avl_insert(tree->left, start_addr, size, used);
    } else if (tree->start_addr < start_addr) {
        tree->right = vmm_avl_insert(tree->right, start_addr, size, used);
    } else {
        return tree;
    }

    tree->height = 1 + max(height(tree->left), height(tree->right));

    int balance = get_balance(tree);

    if (balance > 1 && start_addr < tree->left->start_addr)
        return rotate_right(tree);

    if (balance < -1 && start_addr < tree->right->start_addr)
        return rotate_left(tree);

    if (balance > 1 && start_addr > tree->left->start_addr) {
        tree->left = rotate_left(tree->left);
        return rotate_right(tree);
    }

    if (balance < -1 && start_addr > tree->right->start_addr) {
        tree->right = rotate_right(tree->right);
        return rotate_left(tree);
    }

    return tree;
}

struct vmm_avl* vmm_avl_delete(struct vmm_avl* tree, vaddr_t start_addr) {
    if (tree == NULL)
        return tree;
    
    if (start_addr < tree->start_addr) {
        tree->left = vmm_avl_delete(tree->left, start_addr);
    } else if (start_addr > tree->start_addr) {
        tree->right = vmm_avl_delete(tree->right, start_addr);
    } else {
        if ((tree->left == NULL) && (tree->right == NULL)) {
            kfree(tree);
            tree = NULL;
            return tree;
        } else if (tree->left == NULL) {
            struct vmm_avl* tmp = tree->right;
            kfree(tree);
            tree = tmp;
        } else if (tree->right == NULL) {
            struct vmm_avl* tmp = tree->left;
            kfree(tree);
            tree = tmp;
        } else {
            struct vmm_avl* tmp = get_min_node(tree->right);

            tree->start_addr = tmp->start_addr;
            tree->size = tmp->size;
            tree->used = tmp->used;

            tree->right = vmm_avl_delete(tree->right, tmp->start_addr);
        }
    }

    tree->height = 1 + max(height(tree->left), height(tree->right));

    int balance = get_balance(tree);

    if (balance > 1 && get_balance(tree->left) >= 0) {
        return rotate_right(tree);
    }

    if (balance > 1 && get_balance(tree->left) < 0) {
        tree->left = rotate_left(tree->left);
        return rotate_right(tree);
    }

    if (balance < -1 && get_balance(tree->right) <= 0) {
        return rotate_left(tree);
    }

    if (balance < -1 && get_balance(tree->right) > 0) {
        tree->right = rotate_right(tree->right);
        return rotate_left(tree);
    }

    return tree;
}