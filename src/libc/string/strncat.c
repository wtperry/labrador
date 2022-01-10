#include <string.h>

char* strncat(char* dest, const char* src, size_t n) {
    char* ptr = dest + strlen(dest);

    while (*src != '\0' && n--) {
        *ptr++ = *src++;
    }

    *ptr = '\0';

    return dest;
}