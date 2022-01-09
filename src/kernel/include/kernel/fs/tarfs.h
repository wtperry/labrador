#pragma once

typedef struct tar_hdr {
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char checksum[8];
    char type;
    char link[100];
    char indicator[6];
    char version[2];
    char owner[32];
    char group[32];
    char major[8];
    char minor[8];
    char prefix[155];
} tar_hdr_t;

void tarfs_install(void);
void tarfs_register_tar(const char* name, void* ptr);