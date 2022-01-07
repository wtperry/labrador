#pragma once

#include <kernel/fs/vfs.h>

void dummyfs_install(void);

fs_node_t* dummyfs_mount(const char* device);

size_t dummyfs_read(struct fs_node* node, size_t offset, size_t size, uint8_t* buffer);
dirent_t* dummyfs_readdir(struct fs_node* node, size_t index);
fs_node_t* dummyfs_finddir(struct fs_node* node, const char* name);
size_t dummyfs_getsize(struct fs_node* node);