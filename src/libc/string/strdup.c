#include <string.h>

#include <stdlib.h>

char* strdup(const char* src) {
    char* string = malloc(strlen(src) + 1);
    strcpy(string, src);
    return string;
}

size_t _strnlen(const char* str, size_t n) {
    size_t out = 0;

    while (*str && out < n) {
        out++;
        str++;
    }

    return out;
}

char* strndup(const char* str, size_t size) {
    size = _strnlen(str, size);

    char* out = malloc(size + 1);
    memcpy(out, str, size);
    *(out + size) = '\0';

    return out;
}