#include <libk/string.h>
 
void* memcpy(void* restrict dst, const void* restrict src, size_t size) {
	asm volatile( "cld; rep movsb"
                : "=c"((int){0})
                : "D"(dst), "S"(src), "c"(size)
                : "flags", "memory");
    
    return dst;
}