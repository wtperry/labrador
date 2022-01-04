#include <stdarg.h>
#include <stddef.h>

typedef int (*print_fct_t)(const char* in, void* buffer, size_t idx, size_t num, size_t max_len);

int _vsnprintf(print_fct_t print, char* restrict buffer, const size_t max_len, const char* restrict format, va_list va);