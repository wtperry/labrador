#ifndef _STDIO_H
#define _STDIO_H 1
 
#include <sys/cdefs.h>
#include <stdarg.h>
#include <stddef.h>
 
#define EOF (-1)
 
#ifdef __cplusplus
extern "C" {
#endif
 
int printf(const char* restrict format, ...);
int vprintf(const char* restrict format, va_list args);
int sprintf(char* restrict str, const char* restrict format, ...);
int snprintf(char* restrict str, size_t size, const char* restrict format, ...);
int vsprintf(char* restrict str, const char* restrict format, va_list args);
int putchar(int);
int puts(const char*);
 
#ifdef __cplusplus
}
#endif
 
#endif