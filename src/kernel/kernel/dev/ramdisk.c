#include <kernel/dev/ramdisk.h>

#include <string.h>
#include <stdint.h>
#include <stddef.h>

#include <kernel/heap.h>
#include <kernel/ds/list.h>
#include <kernel/fs/devfs.h>

#define BLOCK_SIZE 0x1000

typedef struct ramdisk {
    char name[256];
    size_t length;
    size_t num_blocks;
    uint8_t** blocks;
} ramdisk_t;

static list_t* ramdisks;

static void ramdisk_expand(ramdisk_t* ramdisk, size_t blocks_to_add) {

}

static size_t ramdisk_read(fs_node_t* node, size_t offset, size_t size, uint8_t* buffer) {
    ramdisk_t* ramdisk = (ramdisk_t*)node->device;

    if (offset >= ramdisk->length) {
        size = 0;
    } else if (offset + size > ramdisk->length) {
        size = ramdisk->length - offset;
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

        memcpy(buffer, ramdisk->blocks[block] + to_read_offset, to_read_size);

        buffer += to_read_size;
    }

    return size;
}

static size_t ramdisk_write(fs_node_t* node, size_t offset, size_t size, const uint8_t* buffer) {
    /*
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
            to_write_size = ((offset + size) % BLOCK_SIZE) - to_write_offset;
        }

        memcpy(file->blocks[block] + to_write_offset, buffer, to_write_size);

        buffer += to_write_size;
    }

    file->length = offset + size;

    return size; */
    return 0;
}

void ramdisk_init(void) {
    ramdisks = list_create();
}

int create_ramdisk_from_address(void* address, size_t size, unsigned int flags, const char* name) {
    ramdisk_t* ramdisk = kmalloc(sizeof(*ramdisk));
    strcpy(ramdisk->name, name);
    ramdisk->num_blocks = (size - 1)/BLOCK_SIZE + 1;
    ramdisk->blocks = kmalloc(sizeof(*ramdisk->blocks) * ramdisk->num_blocks);
    ramdisk->length = size;

    for (size_t i = 0; i < ramdisk->num_blocks; i++) {
        ramdisk->blocks[i] = (uintptr_t)address + BLOCK_SIZE * i;
    }
    
    devfs_device_t* device = kmalloc(sizeof(*device));
    memset(device, 0, sizeof(*device));
    strcpy(device->name, name);
    device->device = ramdisk;
    device->read = &ramdisk_read;
    
    if (!(flags & RAMDISK_READ_ONLY)) {
        device->write = &ramdisk_write;
    }

    return devfs_add_block_device(device, name);
}

void create_new_ramdisk(void) {

}