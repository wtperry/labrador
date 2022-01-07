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

void tree_node_insert_child_node(tree_t* tree, tree_node_t* parent, tree_node_t* child) {
    list_append(parent->children, child);

    tree->nodes++;
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

void tree_set_root(tree_t* tree, void* value) {
    tree_destroy_node(tree->root);

    tree->root = tree_node_create(value);
}

tree_node_t* tree_node_create(void* value) {
    tree_node_t* node = kmalloc(sizeof(tree_node_t));

    node->children = list_create();
    node->parent = NULL;
    node->value = value;

    return node;
}

tree_node_t* tree_node_insert_child(tree_t* tree, tree_node_t* parent, void* value) {
    tree_node_t* child = tree_node_create(value);
    tree_node_insert_child_node(tree, parent, child);

    return child;
}