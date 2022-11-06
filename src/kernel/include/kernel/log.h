#pragma once

#include <stdarg.h>
#include <stddef.h>

#define LOG_FATAL 0
#define LOG_ERROR 1
#define LOG_WARNING 2
#define LOG_INFO 3
#define LOG_DEBUG 4

typedef size_t (*console_write_t)(const char *string);

void log_init(void);

int log_printf(int level, const char* restrict format, ...);
int log_vprintf(int level, const char* restrict format, va_list args);

int log_add_output(int max_level, console_write_t write_func);