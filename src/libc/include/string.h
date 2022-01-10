#ifndef _STRING_H
#define _STRING_H 1
 
#include <sys/cdefs.h>
 
#include <stddef.h>
 
#ifdef __cplusplus
extern "C" {
#endif
 
int memcmp(const void*, const void*, size_t);
void* memcpy(void* __restrict dest, const void* __restrict src, size_t n);
void* memmove(void*, const void*, size_t);
void* memset(void*, int, size_t);
size_t strlen(const char*);
int strcmp(const char* str1, const char* str2);
int strncmp(const char* str1, const char* str2, size_t n);
char* strcpy(char* dest, const char* src);
char* strdup(const char *src);
char* strndup(const char* str, size_t size);
char* strncat(char* dest, const char* src, size_t n);
 
#ifdef __cplusplus
}
#endif
 
#endif