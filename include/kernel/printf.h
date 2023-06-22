#include <stddef.h>
#include <stdarg.h>

int snprintf(char* restrict str, size_t size, const char* restrict format, ...);
int vsnprintf(char* restrict str, size_t size, const char* restrict format, va_list args);