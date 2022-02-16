#include <libk/string.h>

int strcmp(const char* str1, const char* str2) {
    while (*str1) {
        if (*str1 != *str2) {
            break;
        }

        str1++;
        str2++;
    }

    return *(const unsigned char*)str1 - *(const unsigned char*)str2;
}

int strncmp(const char* str1, const char* str2, size_t n) {
    while (n && *str1) {
        if (*str1 != *str2) {
            break;
        }

        str1++;
        str2++;
        n--;
    }

    if (n == 0) {
        return 0;
    } else {
        return *(const unsigned char*)str1 - *(const unsigned char*)str2;
    }
}