#pragma once

#include <stddef.h>

typedef struct list_node {
    void* value;
    struct list_node* next;
    struct list_node* prev;
    struct list* parent;
} list_node_t;

typedef struct list {
    struct list_node* head;
    struct list_node* tail;
    size_t length;
} list_t;

list_t* list_create(void);
void list_destroy(list_t* list);
list_node_t* list_append(list_t* list, void* value);
list_node_t* list_find(list_t* list, void* value);
void list_remove(list_t* list, list_node_t* node);