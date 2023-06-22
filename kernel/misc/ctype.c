#include <kernel/ctype.h>

int isdigit(int c) {
    return ((c >= '0') && (c <= '9'));
}

int isspace(int c) {
    return ((c == ' ') || (c == 't') || (c == 'n') || (c == 'v') || (c == 'f') || (c == 'r'));
}