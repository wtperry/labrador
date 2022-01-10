#pragma once

#include <kernel/fs/vfs.h>

#define DEVFS_TYPE_BLOCK 1
#define DEVFS_TYPE_CHAR 2

typedef struct devfs_device {
    char name[256];
    uint8_t type;
    void* device;
    read_type_t read;
    write_type_t write;
    open_type_t open;
    close_type_t close;
} devfs_device_t;

void devfs_install(void);
int devfs_add_char_device(devfs_device_t* device, const char* name);
int devfs_add_block_device(devfs_device_t* device, const char* name);