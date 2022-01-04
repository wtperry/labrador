#pragma once

#include <stdint.h>
#include <stddef.h>

#define FS_FILE         0x01
#define FS_DIRECTORY    0x02
#define FS_CHARDEVICE   0x04
#define FS_BLOCKDEVICE  0x08
#define FS_PIPE         0x10
#define FS_SYMLINK      0x20
#define FS_MOUNTPOINT   0x40
struct fs_node;

typedef size_t (*read_type_t)(struct fs_node* node, size_t offset, size_t size, uint8_t* buffer);
typedef size_t (*write_type_t)(struct fs_node* node, size_t offset, size_t size, const uint8_t* buffer);
typedef void (*open_type_t)(struct fs_node* node, uint8_t read, uint8_t write);
typedef void (*close_type_t)(struct fs_node* node);

typedef struct fs_node {
    char name[256];
    uint32_t flags;
    size_t length;

    read_type_t read;
    write_type_t write;
    open_type_t open;
    close_type_t close;
} fs_node_t;

char* vfs_canonicize_path(const char* rel_path, char* cwd);