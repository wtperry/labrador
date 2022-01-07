#include <string.h>

#include <stdlib.h>

char* strdup(const char* src) {
    char* string = malloc(strlen(src) + 1);
    strcpy(string, src);
    return string;
}