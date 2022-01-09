#include <kernel/fs/tarfs.h>

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include <kernel/ds/list.h>
#include <kernel/ds/tree.h>
#include <kernel/fs/vfs.h>

typedef struct tarfs {

} tarfs_t;

typedef struct tarfs_node {

} tarfs_node_t;

size_t oct2bin(const char* str, size_t size) {
    size_t n = 0;
    const char *c = str;
    while (size > 0) {
        n *= 8;
        n += *c - '0';
        c++;
        size--;
    }

    return n;
}

size_t tarfs_read(fs_node_t* node, size_t offset, size_t size, uint8_t* buffer) {
    
}

static dirent_t* tarfs_readdir(fs_node_t* node, size_t index) {
    
}

static fs_node_t* tarfs_finddir(fs_node_t* node, const char* name) {
    
}

size_t tarfs_getsize(fs_node_t* node) {

}

static fs_node_t* tarfs_node_from_dir() {
    fs_node_t* node = kmalloc(sizeof(fs_node_t));
    memset(node, 0, sizeof(fs_node_t));

    return node;
}

static fs_node_t* tarfs_node_from_file() {
    fs_node_t* node = kmalloc(sizeof(fs_node_t));
    memset(node, 0, sizeof(fs_node_t));

    return node;
}

static fs_node_t* tarfs_mount(const char* device) {
    
}

void tarfs_install(void) {
    fs_functions_t* funcs = kmalloc(sizeof(fs_functions_t));
    funcs->mount = &tarfs_mount;
    funcs->unmount = NULL;

    vfs_register_fs_driver("tar", funcs);
}