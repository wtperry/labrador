#include <kernel/fs/devfs.h>

#include <libk/string.h>

#include <kernel/heap.h>
#include <kernel/ds/list.h>
#include <kernel/fs/vfs.h>

static list_t* devices;

fs_node_t* devfs_node_from_device(devfs_device_t* device) {
    fs_node_t* node = kmalloc(sizeof(fs_node_t));
    memset(node, 0, sizeof(fs_node_t));
    strcpy(node->name, device->name);
    node->write = device->write;
    node->read = device->read;
    node->open = device->open;
    node->close = device->close;
    node->device = device->device;
    
    if (device->type == DEVFS_TYPE_BLOCK) {
        node->flags = FS_BLOCKDEV;
    } else if (device->type == DEVFS_TYPE_CHAR) {
        node->flags = FS_CHARDEV;
    }


    return node;
}

static dirent_t* devfs_readdir(fs_node_t* node, size_t index) {
    list_t* devices = (list_t*)node->device;

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

    if (index - 2 >= devices->length) {
        return NULL;
    }

    dirent_t* out = kmalloc(sizeof(*out));
    memset(out, 0, sizeof(*out));

    devfs_device_t* device = (devfs_device_t*)list_at(devices, index - 2)->value;
    strcpy(out->name, device->name);

    return out;
}

static fs_node_t* devfs_finddir(fs_node_t* node, const char* name) {
    list_t* devices = (list_t*)node->device;
    list_node_t* curr_list_node = devices->head;

    while(curr_list_node) {
        devfs_device_t* device = (devfs_device_t*)curr_list_node->value;

        if (!strcmp(device->name, name)) {
            return devfs_node_from_device(device);
        }

        curr_list_node = curr_list_node->next;
    }

    return NULL;
}

static fs_node_t* devfs_mount(__attribute__((unused)) const char* device) {
    fs_node_t* devfs_root = kmalloc(sizeof(*devfs_root));
    memset(devfs_root, 0, sizeof(*devfs_root));
    strcpy(devfs_root->name, "[devfs]");
    devfs_root->readdir = &devfs_readdir;
    devfs_root->finddir = &devfs_finddir;
    devfs_root->device = devices;
    devfs_root->flags = FS_DIRECTORY;

    return devfs_root;
}

void devfs_install(void) {
    devices = list_create();

    fs_functions_t* funcs = kmalloc(sizeof(fs_functions_t));
    funcs->mount = &devfs_mount;
    funcs->unmount = NULL;

    vfs_register_fs_driver("devfs", funcs);
    vfs_mount("devfs", "", "/dev");
}

static int devfs_add_device(devfs_device_t* device, const char* name) {
    list_node_t* curr_list_node = (list_node_t*)devices->head;

    while (curr_list_node) {
        devfs_device_t* curr_device = (devfs_device_t*)curr_list_node->value;
        if (!strcmp(curr_device->name, name)) {
            return -1;
        }

        curr_list_node = curr_list_node->next;
    }

    strcpy(device->name, name);
    list_append(devices, device);
    return 0;
}

int devfs_add_char_device(devfs_device_t* device, const char* name) {
    device->type = DEVFS_TYPE_CHAR;
    return devfs_add_device(device, name);
}

int devfs_add_block_device(devfs_device_t* device, const char* name) {
    device->type = DEVFS_TYPE_BLOCK;
    return devfs_add_device(device, name);
}