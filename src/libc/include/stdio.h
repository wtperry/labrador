#ifndef _STDIO_H
#define _STDIO_H 1
 
#include <sys/cdefs.h>
#include <stdarg.h>
#include <stddef.h>
 
#define EOF (-1)
#define SEEK_SET 0
typedef struct { int unused; } FILE;
 
#ifdef __cplusplus
extern "C" {
#endif

extern FILE* stderr;
#define stderr stderr

int fclose(FILE*);
int fflush(FILE*);
FILE *fopen(const char*, const char*);
int fprintf(FILE*, const char*, ...);
size_t fread(void*, size_t, size_t, FILE*);
int fseek(FILE*, long, int);
long ftell(FILE*);
size_t fwrite(const void*, size_t, size_t, FILE*);
void setbuf(FILE*, char*);
int vfprintf(FILE*, const char*, va_list);
 
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