#ifndef _STDLIB_H
#define _STDLIB_H 1
 
#include <libk/sys/cdefs.h>
#include <stddef.h>
 
#ifdef __cplusplus
extern "C" {
#endif
 
__attribute__((__noreturn__))
void abort(void);
int atexit(void (*)(void));
int atoi(const char*);
char *getenv(const char*);
void* malloc(size_t size);
void free(void* ptr);
 
#ifdef __cplusplus
}
#endif
 
#endif