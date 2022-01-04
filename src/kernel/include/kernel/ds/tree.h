#pragma once

#include <stddef.h>

#include <kernel/ds/list.h>

typedef struct tree_node {
    void* value;
    struct tree_node* parent;
    list_t* children;
} tree_node_t;

typedef struct tree {
    struct tree_node* root;
    size_t nodes;
} tree_t;

tree_t* tree_create(void);
void tree_destroy(tree_t* tree);