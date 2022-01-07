#include <kernel/ds/list.h>

#include <stddef.h>

#include <kernel/heap.h>

list_t* list_create(void) {
    list_t* list = kmalloc(sizeof(list_t));
    list->head = NULL;
    list->tail = NULL;
    list->length = 0;

    return list;
}

void list_destroy(list_t* list) {
    list_node_t* node = list->head;

    while(node) {
        list_node_t* next = node->next;
        kfree(node);
        node = next;
    }

    kfree(list);
}

list_node_t* list_append(list_t* list, void* value) {
    list_node_t* node = kmalloc(sizeof(list_node_t));
    node->value = value;
    node->prev = list->tail;
    node->next = NULL;
    node->parent = list;
    
    if (list->tail) {
        list->tail->next = node;
    } else {
        list->head = node;
    }

    list->tail = node;
    list->length++;

    return node;
}

list_node_t* list_find(list_t* list, void* value) {
    list_node_t* node = list->head;

    while (node) {
        if (node->value == value) {
            return node;
        }

        node = node->next;
    }

    return NULL;
}

void list_remove(list_t* list, list_node_t* node) {
    if (node == list->head) {
        list->head = node->next;
    }
    if (node == list->tail) {
        list->tail = node->prev;
    }
    if (node->prev) {
        node->prev->next = node->next;
    }
    if (node->next) {
        node->next->prev = node->prev;
    }

    list->length--;

    kfree(node);
}