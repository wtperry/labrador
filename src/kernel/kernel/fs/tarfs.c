#include <kernel/fs/tarfs.h>

#include <stddef.h>
#include <stdint.h>
#include <libk/string.h>
#include <libk/stdio.h>
#include <stdbool.h>

#include <kernel/heap.h>
#include <kernel/ds/list.h>
#include <kernel/ds/tree.h>
#include <kernel/fs/vfs.h>

typedef struct tarfs {
    fs_node_t* tar_file;
} tarfs_t;

static fs_node_t* tarfs_node_from_hdr(tar_hdr_t* hdr, fs_node_t* tar_file, size_t offset);

static size_t oct2bin(const char* str, size_t size) {
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

static size_t count_slashes(const char* str) {
    size_t n = 0;
    while (*str) {
        if (*str == '/') {
            if (*(str + 1) == '\0') {
                return n;
            }

            n++;
        }

        str++;
    }

    return n;
}

static size_t tarfs_read(fs_node_t* node, size_t offset, size_t size, uint8_t* buffer) {
    fs_node_t* tar_file = (fs_node_t*)node->device;

    tar_hdr_t hdr;
    vfs_read(tar_file, node->inode_no, sizeof(tar_hdr_t), (uint8_t*)&hdr);
    size_t file_size = oct2bin(hdr.size, 11);

    if (offset >= file_size) {
        size = 0;
    } else if (offset + size > file_size) {
        size = file_size - offset;
    }

    return vfs_read(tar_file, node->inode_no + TAR_BLOCK_SIZE + offset, size, buffer);
}

static dirent_t* tarfs_readdir_root(fs_node_t* node, size_t index) {
    fs_node_t* tar_file = (fs_node_t*)node->device;

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

    index -= 2;

    size_t curr_offset = 0;
    size_t to_find = index + 1;
    tar_hdr_t hdr;

    while (vfs_read(tar_file, curr_offset, sizeof(tar_hdr_t), (uint8_t*)&hdr)) {
        char filename[256];
        filename[0] = '\0';
        strncat(filename, hdr.prefix, 155);
        strncat(filename, hdr.name, 100);
        if (!filename[0]) {
            break;
        }
        if (!count_slashes(filename) && strcmp(filename, "")) {
            to_find--;

            if (!to_find) {
                dirent_t* out = kmalloc(sizeof(*out));
                memset(out, 0, sizeof(*out));
                char* ptr = filename + strlen(filename) - 1;
                if (*ptr == '/') {
                    *ptr-- = '\0';
                }
                while (*ptr != '/' && ptr > filename) {
                    ptr--;
                }
                if (*ptr == '/') {
                    ptr++;
                }

                strcpy(out->name, ptr);
                return out;
            }
        }

        size_t size = oct2bin(hdr.size, 11);
        curr_offset += TAR_BLOCK_SIZE + ((size-1)/TAR_BLOCK_SIZE + 1) * TAR_BLOCK_SIZE;
    }

    return NULL;
}

static dirent_t* tarfs_readdir(fs_node_t* node, size_t index) {
    fs_node_t* tar_file = (fs_node_t*)node->device;

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

    index -= 2;

    size_t curr_offset = 0;
    size_t to_find = index + 1;
    tar_hdr_t hdr;

    char dir_name[256];
    dir_name[0] = '\0';

    vfs_read(tar_file, node->inode_no, sizeof(tar_hdr_t), (uint8_t*)&hdr);

    strncat(dir_name, hdr.prefix, 155);
    strncat(dir_name, hdr.name, 100);

    size_t dir_name_len = strlen(dir_name);
    size_t dir_depth = count_slashes(dir_name);

    while (vfs_read(tar_file, curr_offset, sizeof(tar_hdr_t), (uint8_t*)&hdr)) {
        char filename[256];
        filename[0] = '\0';
        strncat(filename, hdr.prefix, 155);
        strncat(filename, hdr.name, 100);
        if (!filename[0]) {
            break;
        }
        if (count_slashes(filename) == dir_depth + 1 && !memcmp(dir_name, filename, dir_name_len)) {
            to_find--;

            if (!to_find) {
                dirent_t* out = kmalloc(sizeof(*out));
                memset(out, 0, sizeof(*out));
                char* ptr = filename + strlen(filename) - 1;
                if (*ptr == '/') {
                    *ptr-- = '\0';
                }
                while (*ptr != '/' && ptr > filename) {
                    ptr--;
                }
                if (*ptr == '/') {
                    ptr++;
                }

                strcpy(out->name, ptr);
                return out;
            }
        }

        size_t size = oct2bin(hdr.size, 11);
        curr_offset += TAR_BLOCK_SIZE + ((size-1)/TAR_BLOCK_SIZE + 1) * TAR_BLOCK_SIZE;
    }

    return NULL;
}

static fs_node_t* tarfs_finddir_root(fs_node_t* node, const char* name) {
    fs_node_t* tar_file = (fs_node_t*)node->device;

    size_t curr_offset = 0;
    tar_hdr_t hdr;

    while (vfs_read(tar_file, curr_offset, sizeof(tar_hdr_t), (uint8_t*)&hdr)) {
        char filename[256];
        filename[0] = '\0';
        strncat(filename, hdr.prefix, 155);
        strncat(filename, hdr.name, 100);
        if (!filename[0]) {
            break;
        }
        if (!memcmp(filename, name, strlen(name))) {
            return tarfs_node_from_hdr(&hdr, tar_file, curr_offset);
        }

        size_t size = oct2bin(hdr.size, 11);
        curr_offset += TAR_BLOCK_SIZE + ((size-1)/TAR_BLOCK_SIZE + 1) * TAR_BLOCK_SIZE;
    }

    return NULL;
}

static fs_node_t* tarfs_finddir(fs_node_t* node, const char* name) {
    fs_node_t* tar_file = (fs_node_t*)node->device;

    size_t curr_offset = 0;
    tar_hdr_t hdr;

    char dir_name[256];
    dir_name[0] = '\0';

    vfs_read(tar_file, node->inode_no, sizeof(tar_hdr_t), (uint8_t*)&hdr);

    strncat(dir_name, hdr.prefix, 155);
    strncat(dir_name, hdr.name, 100);
    strncat(dir_name, name, 256);

    while (vfs_read(tar_file, curr_offset, sizeof(tar_hdr_t), (uint8_t*)&hdr)) {
        char filename[256];
        filename[0] = '\0';
        strncat(filename, hdr.prefix, 155);
        strncat(filename, hdr.name, 100);
        if (!filename[0]) {
            break;
        }
        if (!memcmp(filename, dir_name, strlen(dir_name))) {
            return tarfs_node_from_hdr(&hdr, tar_file, curr_offset);
        }

        size_t size = oct2bin(hdr.size, 11);
        curr_offset += TAR_BLOCK_SIZE + ((size-1)/TAR_BLOCK_SIZE + 1) * TAR_BLOCK_SIZE;
    }

    return NULL;
}

size_t tarfs_getsize(fs_node_t* node) {

}

static fs_node_t* tarfs_node_from_hdr(tar_hdr_t* hdr, fs_node_t* tar_file, size_t offset) {
    fs_node_t* node = kmalloc(sizeof(*node));
    memset(node, 0, sizeof(*node));
    strncat(node->name, hdr->prefix, 155);
    strncat(node->name, hdr->name, 100);
    if (hdr->type == TAR_TYPE_FILE || hdr->type == TAR_TYPE_AFILE) {
        node->flags = FS_FILE;
        node->read = &tarfs_read;
    } else if (hdr->type == TAR_TYPE_DIR) {
        node->flags = FS_DIRECTORY;
        node->name[strlen(node->name) - 1] = '\0';
        node->finddir = &tarfs_finddir;
        node->readdir = &tarfs_readdir;
    }

    node->device = tar_file;
    node->inode_no = offset;

    char* ptr = node->name + strlen(node->name) - 1;
    while (ptr > node->name) {
        if (*ptr == '/') {
            ptr++;
            break;
        }

        ptr--;
    }

    memmove(node->name, ptr, strlen(ptr) + 1);

    return node;
}

static fs_node_t* tarfs_mount(const char* device) {
    fs_node_t* tar_file = vfs_get_fs_node(device);

    if (!tar_file) {
        return NULL;
    }

    fs_node_t* tarfs_node = kmalloc(sizeof(*tarfs_node));
    memset(tarfs_node, 0, sizeof(*tarfs_node));
    strcpy(tarfs_node->name, "[tarfs]");
    tarfs_node->device = tar_file;
    tarfs_node->flags = FS_DIRECTORY;
    tarfs_node->finddir = &tarfs_finddir_root;
    tarfs_node->readdir = &tarfs_readdir_root;

    return tarfs_node;
}

void tarfs_install(void) {
    fs_functions_t* funcs = kmalloc(sizeof(fs_functions_t));
    funcs->mount = &tarfs_mount;
    funcs->unmount = NULL;

    vfs_register_fs_driver("tar", funcs);
}