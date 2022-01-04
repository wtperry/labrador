#include <kernel/ds/tree.h>

#include <stddef.h>

#include <kernel/heap.h>

void tree_destroy_node(tree_node_t* node) {
    if (!node) {
        return;
    }

    list_node_t* child = node->children->head;

    while (child) {
        tree_destroy_node(child->value);
        child = child->next;
    }

    list_destroy(node->children);

    kfree(node);
}

tree_t* tree_create(void) {
    tree_t* tree = kmalloc(sizeof(tree_t));
    tree->nodes = 0;
    tree->root = NULL;

    return tree;
}

void tree_destroy(tree_t* tree) {
    tree_destroy_node(tree->root);

    kfree(tree);
}