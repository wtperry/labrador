#include <kernel/fs/tmpfs.h>

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <kernel/fs/vfs.h>
#include <kernel/ds/list.h>
#include <kernel/vmm.h>
#include <kernel/heap.h>

#define TMPFS_TYPE_FILE 1
#define TMPFS_TYPE_DIR 2

#define BLOCK_SIZE 0x1000   // 4 KB (one page)

static list_t* free_blocks;

static fs_node_t* tmpfs_node_from_dir(tmpfs_dir_t* dir);
static fs_node_t* tmpfs_node_from_file(tmpfs_file_t* file);

size_t tmpfs_read(fs_node_t* node, size_t offset, size_t size, uint8_t* buffer) {
    tmpfs_file_t* file = (tmpfs_file_t*)node->device;

    if (offset >= file->length) {
        size = 0;
    } else if (offset + size > file->length) {
        size = file->length - offset;
    }

    size_t start_block = offset / BLOCK_SIZE;
    size_t end_block = (offset + size - 1) / BLOCK_SIZE;
    
    for (size_t block = start_block; block <= end_block; block++) {
        size_t to_read_size = BLOCK_SIZE;
        size_t to_read_offset = 0;
        if (block == start_block) {
            to_read_offset = offset % BLOCK_SIZE;
            to_read_size -= to_read_offset;
        }
        if (block == end_block) {
            to_read_size = ((offset + size) % BLOCK_SIZE) - to_read_offset;
        }

        memcpy(buffer, file->blocks[block] + to_read_offset, to_read_size);

        buffer += to_read_size;
    }

    return size;
}

size_t tmpfs_write(fs_node_t* node, size_t offset, size_t size, const uint8_t* buffer) {
    tmpfs_file_t* file = (tmpfs_file_t*)node->device;

    size_t new_num_blocks = ((offset + size - 1) / SIZE_MAX) + 1;

    if (file->num_blocks < new_num_blocks) {
        uint8_t** new_blocks = kmalloc(sizeof(*new_blocks) * new_num_blocks);
        memcpy(new_blocks, file->blocks, file->num_blocks * sizeof(*new_blocks));

        for (size_t i = file->num_blocks; i < new_num_blocks; i++) {
            new_blocks[i] = (uint8_t*)vmm_alloc_region(BLOCK_SIZE / 4096);
        }

        kfree(file->blocks);
        file->blocks = new_blocks;
        file->num_blocks = new_num_blocks;
    }

    size_t start_block = offset / BLOCK_SIZE;
    size_t end_block = (offset + size - 1) / BLOCK_SIZE;
    
    for (size_t block = start_block; block <= end_block; block++) {
        size_t to_write_size = BLOCK_SIZE;
        size_t to_write_offset = 0;
        if (block == start_block) {
            to_write_offset = offset % BLOCK_SIZE;
            to_write_size -= to_write_offset;
        }
        if (block == end_block) {
            to_write_size = (offset + size) % BLOCK_SIZE - to_write_offset;
        }

        memcpy(file->blocks[block] + to_write_offset, buffer, to_write_size);

        buffer += to_write_size;
    }

    file->length = offset + size;

    return size;
}

static dirent_t* tmpfs_readdir(fs_node_t* node, size_t index) {
    tmpfs_dir_t* dir = (tmpfs_dir_t*)node->device;

    if (index == 0) {
        dirent_t* out = kmalloc(sizeof(*out));
        memset(out, 0, sizeof(*out));
        strcpy(out->name, ".");
        return out;
    }

    if (index == 1) {
        dirent_t* out = kmalloc(sizeof(*out));
        memset(out, 0, sizeof(*out));
        strcpy(out->name, "..");
        return out;
    }

    if (index - 2 >= dir->files->length) {
        return NULL;
    }

    dirent_t* out = kmalloc(sizeof(*out));
    memset(out, 0, sizeof(*out));

    tmpfs_entry_t* entry = (tmpfs_entry_t*)list_at(dir->files, index - 2)->value;
    strcpy(out->name, entry->name);

    return out;
}

static fs_node_t* tmpfs_finddir(fs_node_t* node, const char* name) {
    tmpfs_dir_t* dir = (tmpfs_dir_t*)node->device;

    list_node_t* curr_list_node = dir->files->head;

    while(curr_list_node) {
        tmpfs_entry_t* entry = (tmpfs_entry_t*)curr_list_node->value;

        if (!strcmp(entry->name, name)) {
            if (entry->type == TMPFS_TYPE_DIR) {
                return tmpfs_node_from_dir((tmpfs_dir_t*)entry);
            } else if (entry->type == TMPFS_TYPE_FILE) {
                return tmpfs_node_from_file((tmpfs_file_t*)entry);
            }
        }

        curr_list_node = curr_list_node->next;
    }

    return NULL;
}

static int tmpfs_mkdir(fs_node_t* node, const char* name) {
    tmpfs_dir_t* parent_dir = (tmpfs_dir_t*)node->device;

    tmpfs_dir_t* new_dir = kmalloc(sizeof(*new_dir));
    new_dir->name = strdup(name);
    new_dir->files = list_create();
    new_dir->parent = parent_dir;
    new_dir->type = TMPFS_TYPE_DIR;

    list_append(parent_dir->files, new_dir);

    return 0;
}

int tmpfs_create(fs_node_t* node, const char* name) {
    tmpfs_dir_t* dir = (tmpfs_dir_t*)node->device;

    tmpfs_file_t* new_file = kmalloc(sizeof(*new_file));
    new_file->name = strdup(name);
    new_file->type = TMPFS_TYPE_FILE;
    new_file->parent = dir;
    new_file->length = 0;
    new_file->blocks = NULL;
    new_file->num_blocks = 0;

    list_append(dir->files, new_file);

    return 0;
}

int tmpfs_remove(fs_node_t* node) {
    tmpfs_entry_t* entry = (tmpfs_entry_t*)node->device;

    tmpfs_dir_t* parent = entry->parent;
    if (!parent) {
        return -1;
    }

    if (entry->type == TMPFS_TYPE_DIR) {
        tmpfs_dir_t* dir = (tmpfs_dir_t*)entry;
        if (dir->files->length > 0) {
            return -1;
        }
    }

    if (entry->type == TMPFS_TYPE_FILE) {
        tmpfs_file_t* file = (tmpfs_file_t*)file;

        if (file->blocks) {
            for (size_t i = 0; i < file->num_blocks; i++) {
                list_append(free_blocks, file->blocks[i]);
            }

            kfree(file->blocks);
        }
    }

    list_node_t* list_node = list_find(parent->files, entry);
    list_remove(parent->files, list_node);

    kfree(entry);

    return 0;
}

size_t tmpfs_getsize(fs_node_t* node) {
    tmpfs_file_t* file = (tmpfs_file_t*)node->device;
    return file->length;
}

static fs_node_t* tmpfs_node_from_dir(tmpfs_dir_t* dir) {
    fs_node_t* node = kmalloc(sizeof(fs_node_t));
    memset(node, 0, sizeof(fs_node_t));
    strcpy(node->name, dir->name);
    node->readdir = &tmpfs_readdir;
    node->finddir = &tmpfs_finddir;
    node->mkdir = &tmpfs_mkdir;
    node->remove = &tmpfs_remove;
    node->create = &tmpfs_create;
    node->device = dir;
    node->flags = FS_DIRECTORY;

    return node;
}

static fs_node_t* tmpfs_node_from_file(tmpfs_file_t* file) {
    fs_node_t* node = kmalloc(sizeof(fs_node_t));
    memset(node, 0, sizeof(fs_node_t));
    strcpy(node->name, file->name);
    node->write = &tmpfs_write;
    node->read = &tmpfs_read;
    node->open = NULL;
    node->close = NULL;
    node->getsize = &tmpfs_getsize;
    node->remove = &tmpfs_remove;
    node->device = file;
    node->flags = FS_FILE;

    return node;
}

static fs_node_t* tmpfs_mount(const char* device) {
    tmpfs_dir_t* tmpfs = kmalloc(sizeof(*tmpfs));
    tmpfs->name = strdup(device);
    tmpfs->files = list_create();
    tmpfs->parent = NULL;

    return tmpfs_node_from_dir(tmpfs);
}

void tmpfs_install(void) {
    free_blocks = list_create();

    fs_functions_t* funcs = kmalloc(sizeof(fs_functions_t));
    funcs->mount = &tmpfs_mount;
    funcs->unmount = NULL;

    vfs_register_fs_driver("tmpfs", funcs);
}