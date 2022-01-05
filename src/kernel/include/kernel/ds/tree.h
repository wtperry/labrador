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
void tree_set_root(tree_t* tree, void* value);
tree_node_t* tree_node_create(void* value);
tree_node_t* tree_node_insert_child(tree_t* tree, tree_node_t* parent, void* value);