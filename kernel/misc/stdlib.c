#include <kernel/stdlib.h>

#include <kernel/ctype.h>

#include <stddef.h>

int atoi(const char* str) {
    int i = 0;
    size_t index = 0;
    int mult = 1;

    while(isspace(str[index])) {
        index++;
    }

    if(str[index] == '-') {
        mult = -1;
        index++;
    } else if (str[index] == '+') {
        index++;
    }

    while(isdigit(str[index])) {
        i = i * 10 + str[index] - '0';
        index++;
    }

    i *= mult;

    return i;
}