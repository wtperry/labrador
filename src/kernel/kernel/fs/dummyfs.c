#include <kernel/fs/dummyfs.h>

#include <string.h>

#include <kernel/heap.h>
#include <kernel/fs/vfs.h>

void dummyfs_install(void) {
    fs_functions_t* funcs = kmalloc(sizeof(fs_functions_t));
    funcs->mount = &dummyfs_mount;
    funcs->unmount = NULL;

    vfs_register_fs_driver("dummyfs", funcs);
}

fs_node_t* dummyfs_mount(const char* device) {
    fs_node_t* node = kmalloc(sizeof(fs_node_t));
    memset(node, 0, sizeof(fs_node_t));
    strcpy(node->name, "testdir");
    node->readdir = &dummyfs_readdir;
    node->finddir = &dummyfs_finddir;

    return node;
}

size_t dummyfs_read(struct fs_node* node, size_t offset, size_t size, uint8_t* buffer) {
    (void)offset;
    
    const char* msg = "hello vfs world!";
    if (size > strlen(msg)) {
        size = strlen(msg);
    }
    
    for (size_t i = 0; i < size; i++) {
        buffer[i] = msg[i];
    }

    return size;
}
dirent_t* dummyfs_readdir(struct fs_node* node, size_t index) {
    dirent_t* out;
    
    switch (index) {
        case 0:
            out = kmalloc(sizeof(dirent_t));
            memset(out, 0, sizeof(dirent_t));
            strcpy(out->name, ".");
            return out;
            break;
        case 1:
            out = kmalloc(sizeof(dirent_t));
            memset(out, 0, sizeof(dirent_t));
            strcpy(out->name, "..");
            return out;
            break;
        case 2:
            out = kmalloc(sizeof(dirent_t));
            memset(out, 0, sizeof(dirent_t));
            strcpy(out->name, "test1");
            return out;
            break;
        case 3:
            out = kmalloc(sizeof(dirent_t));
            memset(out, 0, sizeof(dirent_t));
            strcpy(out->name, "test2");
            return out;
            break;
    }

    return NULL;
}

fs_node_t* dummyfs_finddir(struct fs_node* node, const char* name) {
    fs_node_t* file = kmalloc(sizeof(fs_node_t));
    memset(file, 0, sizeof(fs_node_t));

    strcpy(file->name, name);
    file->read = &dummyfs_read;

    return file;
}

size_t dummyfs_getsize(struct fs_node* node) {
    return 10;
}