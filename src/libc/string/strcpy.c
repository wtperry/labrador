#include <libk/string.h>

char* strcpy(char* dest, const char* src) {
    memcpy(dest, src, strlen(src) + 1);
    return dest;
}