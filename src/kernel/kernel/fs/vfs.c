#include <kernel/fs/vfs.h>

#include <libk/string.h>
#include <libk/stdio.h>

#include <kernel/heap.h>
#include <kernel/ds/tree.h>
#include <kernel/ds/list.h>

tree_t* fs_tree;
list_t* fs_drivers;

// path needs to be delimeted by '\0' for directories
tree_node_t* get_deepest_tree_node(char* path, size_t depth, char** out_ptr, size_t* out_depth) {
    tree_node_t* curr_node = fs_tree->root;
    char* curr_path = path + 1;
    size_t curr_depth;

    for (curr_depth = 0; curr_depth < depth; curr_depth++) {
        int found = 0;
        list_node_t* child = curr_node->children->head;
        while (child) {
            tree_node_t* child_node = child->value;
            vfs_entry_t* curr_entry = child_node->value;
            if (!strcmp(curr_path, curr_entry->name)) {
                curr_node = child_node;
                found = 1;
                curr_path += strlen(curr_path) + 1;
                break;
            }
            child = child->next;
        }

        if (!found) {
            break;
        }
    }

    *out_ptr = curr_path;
    *out_depth = curr_depth;

    return curr_node;
}

size_t split_path_and_get_depth(char* path) {
    size_t depth = 0;
    size_t path_len = strlen(path);

    if (!strcmp(path, "/")) {
        *path = '\0';
        return 0;
    }

    for (size_t i = 0; i < path_len; i++) {
        if (path[i] == '/') {
            path[i] = '\0';
            depth++;
        }
    }

    return depth;
}

fs_driver_t* get_driver(const char* fs_name) {
    for (list_node_t* list_item = fs_drivers->head; list_item; list_item = list_item->next) {
        fs_driver_t* driver = list_item->value;
        if (!strcmp(driver->fs_name, fs_name)) {
            return driver;
        }
    }

    return NULL;
}

void vfs_init() {
    fs_tree = tree_create();
    vfs_entry_t* root_entry = kmalloc(sizeof(vfs_entry_t));
    root_entry->file = NULL;
    root_entry->name = NULL;
    tree_set_root(fs_tree, root_entry);

    fs_drivers = list_create();
}

void vfs_register_fs_driver(const char* fs_name, const fs_functions_t* funcs) {
    fs_driver_t* fs_driver = kmalloc(sizeof(fs_driver_t));
    fs_driver->fs_name = kmalloc(strlen(fs_name) + 1);
    memcpy(fs_driver->fs_name, fs_name, strlen(fs_name) + 1);
    fs_driver->fs_functions = funcs;

    list_append(fs_drivers, fs_driver);
}

int vfs_mount(const char* fs_type, const char* device, const char* mount_path) {
    if (!strcmp(mount_path, "/")) {
        vfs_entry_t* root_entry = fs_tree->root->value;
        if (root_entry->file) {
            return -1;
        }
    } else {
        fs_node_t* mount_node = vfs_get_fs_node(mount_path);

        if (!mount_node) {
            return -1;
        }

        if (!(mount_node->flags & FS_DIRECTORY)) {
            kfree(mount_node);
            return -1;
        }

        kfree(mount_node);
    }

    fs_driver_t* fs_driver = get_driver(fs_type);

    if (!fs_driver) {
        return -1;
    }

    fs_node_t* mount_root = fs_driver->fs_functions->mount(device);

    if (!mount_root) {
        return -1;
    }

    char* split_path = strdup(mount_path);
    size_t depth = split_path_and_get_depth(split_path);

    char* curr_path = NULL;
    size_t curr_depth = 0;

    tree_node_t* curr_node = get_deepest_tree_node(split_path, depth, &curr_path, &curr_depth);

    while (curr_depth < depth) {
        vfs_entry_t* curr_entry = curr_node->value;
        fs_node_t* new_file = vfs_finddir(curr_entry->file, curr_path);

        vfs_entry_t* new_entry = kmalloc(sizeof(vfs_entry_t));
        new_entry->file = new_file;
        new_entry->name = strdup(new_file->name);
        
        curr_node = tree_node_insert_child(fs_tree, curr_node, new_entry);

        curr_depth++;
        curr_path += strlen(curr_path);
    }

    vfs_entry_t* mount_entry = curr_node->value;
    kfree(mount_entry->file);
    mount_entry->file = mount_root;

    if (!strcmp(mount_path, "/")) {
        kfree(mount_entry->name);
        mount_entry->name = strdup("[root]");
    }

    return 0;
}

fs_node_t* vfs_get_fs_node(const char* path) {
    size_t path_len = strlen(path);
    char* split_path = kmalloc(path_len + 1);
    memcpy(split_path, path, path_len + 1);

    size_t depth = split_path_and_get_depth(split_path);

    char* curr_path = NULL;
    size_t curr_depth = 0;

    vfs_entry_t* mount_entry = get_deepest_tree_node(split_path, depth, &curr_path, &curr_depth)->value;
    fs_node_t* mount_node = mount_entry->file;

    if (!mount_node) {
        return NULL;
    }

    fs_node_t* curr_fs_node = kmalloc(sizeof(*curr_fs_node));
    memcpy(curr_fs_node, mount_node, sizeof(fs_node_t));

    for (; curr_depth < depth; curr_depth++) {
        fs_node_t* new_fs_node = vfs_finddir(curr_fs_node, curr_path);

        kfree(curr_fs_node);
        curr_fs_node = new_fs_node;

        if (!curr_fs_node) {
            return NULL;
        }

        curr_path += (strlen(curr_path) + 1);
    }

    kfree(split_path);

    return curr_fs_node;
}

char* vfs_canonicize_path(const char* path, char* cwd) {
    char* abs_path;

    if (path[0] != '/') {
        //path is relative
        abs_path = kmalloc(strlen(cwd) + strlen(path) + 2);
        snprintf(abs_path, strlen(path) + strlen(cwd) + 2, "%s/%s", cwd, path);
    } else {
        //path is absolute
        abs_path = kmalloc(strlen(path) + 1);
        memcpy(abs_path, path, strlen(path) + 1);
    }

    size_t path_len = strlen(abs_path);
    char* ptr = abs_path + path_len - 1;

    char* new_path = kmalloc(path_len + 1);
    char* nptr = new_path + path_len;
    *nptr = '\0';

    int skip = 0;

    for (; ptr >= abs_path; ptr--) {
        if (*ptr != '/') {
            continue;
        }

        *ptr = '\0';    //change / to null to make copy and compare easier
        char* dir = ptr + 1;

        // Ignore multiple slashes or slash at end of path
        if (!*dir) {
            continue;
        }

        // Ignore . dir
        if (!strcmp(dir, ".")) {
            continue;
        }

        // Skip parent directory when .. dir found
        if (!strcmp(dir, "..")) {
            skip++;
            continue;
        }

        // Skip this directory if .. was found before
        if (skip) {
            skip--;
            continue;
        }

        // Edge cases dealt with, move dir to new_path
        size_t dir_len = strlen(dir);
        nptr -= dir_len;
        memcpy(nptr, dir, dir_len);
        nptr--;
        *nptr = '/';
    }

    // If nothing added to the string, add the / for the root directory
    if (!*nptr) {
        *(--nptr) = '/';
    }

    kfree(abs_path);

    memmove(new_path, nptr, strlen(nptr) + 1);
    return new_path;
}

size_t vfs_read(struct fs_node* node, size_t offset, size_t size, uint8_t* buffer) {
    if (node) {
        if (node->read) {
            return node->read(node, offset, size, buffer);
        }
    }

    //TODO: Add error codes
    return -1;
}
size_t vfs_write(struct fs_node* node, size_t offset, size_t size, const uint8_t* buffer) {
    if (node) {
        if (node->write) {
            return node->write(node, offset, size, buffer);
        }
    }

    //TODO: Add error codes
    return -1;
}

void vfs_open(struct fs_node* node, uint64_t flags) {
    if (!node) {
        return;
    }

    if (node->ref_count != SIZE_MAX) {
        node->ref_count++;
    }

    if (node->open) {
        node->open(node, flags);
    }
}

void vfs_close(struct fs_node* node) {
    if (node) {
        if (node->close) {
            node->close(node);
        }
    }
}
struct dirent* vfs_readdir(struct fs_node* node, size_t index) {
    if (node) {
        if (node->readdir) {
            return node->readdir(node, index);
        }
    }

    //TODO: Add error codes
    return NULL;
}

struct fs_node* vfs_finddir(struct fs_node* node, const char* name) {
    if (node) {
        if (node->finddir) {
            return node->finddir(node, name);
        }
    }

    //TODO: Add error codes
    return NULL;
}

int vfs_create(const char* path) {
    if (!path) {
        return -1;
    }

    if (!strlen(path)) {
        return -1;
    }

    if (*path != '/') {
        return -1;
    }

    // Split parent dir path and dir name
    char* parent_path = strdup(path);
    char* name = parent_path + strlen(parent_path) - 1;
    while (*name != '/') {
        name--;
    }

    *name = '\0';
    name++;

    fs_node_t* parent;
    if (*parent_path == '\0') {
        parent = vfs_get_fs_node("/");
    } else {
        parent = vfs_get_fs_node(parent_path);
    }
    kfree(parent_path);

    if (!parent) {
        return -1;
    }

    if (!parent->create) {
        return -1;
    }

    int retval = parent->create(parent, name);
    kfree(parent);

    return retval;
}

int vfs_remove(const char* path) {
    (void)path;
    //TODO: Add error codes
    return -1;
}

int vfs_mkdir(const char* path) {
    if (!path) {
        return -1;
    }

    if (!strlen(path)) {
        return -1;
    }

    if (*path != '/') {
        return -1;
    }

    // Split parent dir path and dir name
    char* parent_path = strdup(path);
    char* name = parent_path + strlen(parent_path) - 1;
    while (*name != '/') {
        name--;
    }

    *name = '\0';
    name++;

    fs_node_t* parent;
    if (*parent_path == '\0') {
        parent = vfs_get_fs_node("/");
    } else {
        parent = vfs_get_fs_node(parent_path);
    }

    if (!parent) {
        return -1;
    }

    if (!parent->mkdir) {
        kfree(parent_path);
        kfree(parent);
        return -1;
    }

    int retval = parent->mkdir(parent, name);
    kfree(parent_path);
    kfree(parent);

    return retval;
}

size_t vfs_getsize(struct fs_node* node) {
    if (node) {
        if (node->getsize) {
            return node->getsize(node);
        }
    }

    //TODO: Add error codes
    return -1;
}