#pragma once

#include <stdint.h>
#include <stddef.h>

#define FS_FILE         0x01
#define FS_DIRECTORY    0x02
#define FS_CHARDEVICE   0x04
#define FS_BLOCKDEVICE  0x08
#define FS_PIPE         0x10
#define FS_SYMLINK      0x20

struct fs_node;

typedef struct dirent {
    char name[256];
} dirent_t;

typedef size_t (*read_type_t)(struct fs_node* node, size_t offset, size_t size, uint8_t* buffer);
typedef size_t (*write_type_t)(struct fs_node* node, size_t offset, size_t size, const uint8_t* buffer);
typedef void (*open_type_t)(struct fs_node* node, uint64_t flags);
typedef void (*close_type_t)(struct fs_node* node);
typedef struct dirent* (*readdir_type_t)(struct fs_node* node, size_t index);
typedef struct fs_node* (*finddir_type_t)(struct fs_node* node, const char* name);
typedef int (*create_type_t)(struct fs_node* node, const char* name);
typedef int (*delete_type_t)(struct fs_node* node);
typedef int (*mkdir_type_t)(struct fs_node* node, const char* name);
typedef size_t (*getsize_type_t)(struct fs_node* node);

typedef struct fs_node {
    char name[256];
    uint32_t flags;
    size_t length;

    size_t ref_count;

    void* device;

    read_type_t read;
    write_type_t write;
    open_type_t open;
    close_type_t close;
    readdir_type_t readdir;
    finddir_type_t finddir;
    create_type_t create;
    delete_type_t delete;
    mkdir_type_t mkdir;
    getsize_type_t getsize;
} fs_node_t;

typedef struct vfs_entry {
    char* name;
    fs_node_t* file;
} vfs_entry_t;

typedef fs_node_t* (*mount_type_t)(const char* device);
typedef int (*unmount_type_t)(fs_node_t* node);

typedef struct fs_functions {
    mount_type_t mount;
    unmount_type_t unmount;
} fs_functions_t;

typedef struct fs_driver {
    char* fs_name;
    const fs_functions_t* fs_functions;
} fs_driver_t;

void vfs_init(void);

void vfs_register_fs_driver(const char* fs_name, const fs_functions_t* funcs);
int vfs_mount(const char* fs_type, const char* device, const char* mount_path);
fs_node_t* vfs_get_fs_node(const char* path);
char* vfs_canonicize_path(const char* rel_path, char* cwd);

size_t vfs_read(struct fs_node* node, size_t offset, size_t size, uint8_t* buffer);
size_t vfs_write(struct fs_node* node, size_t offset, size_t size, const uint8_t* buffer);
void vfs_open(struct fs_node* node, uint64_t flags);
void vfs_close(struct fs_node* node);
struct dirent* vfs_readdir(struct fs_node* node, size_t index);
struct fs_node* vfs_finddir(struct fs_node* node, const char* name);
int vfs_create(const char* path);
int vfs_delete(const char* path);
int vfs_mkdir(const char* path);
size_t vfs_getsize(struct fs_node* node);