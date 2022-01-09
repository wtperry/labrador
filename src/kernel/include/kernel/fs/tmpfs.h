#pragma once

#include <stddef.h>
#include <stdint.h>

#include <kernel/fs/vfs.h>
#include <kernel/ds/list.h>

typedef struct tmpfs_entry {
    char* name;
    int type;
    struct tmpfs_dir* parent;
} tmpfs_entry_t;

typedef struct tmpfs_file {
    char* name;
    int type;
    struct tmpfs_dir* parent;
    size_t length;
    size_t num_blocks;
    uint8_t** blocks;
} tmpfs_file_t;

typedef struct tmpfs_dir {
    char* name;
    int type;
    struct tmpfs_dir* parent;
    list_t* files;
} tmpfs_dir_t;

void tmpfs_install(void);