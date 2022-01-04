#include <kernel/fs/vfs.h>
#include <kernel/heap.h>

#include <string.h>
#include <stdio.h>

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