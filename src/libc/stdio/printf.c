#include <limits.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "_vsnprintf.h"

int _print(const char* in, void* buffer, size_t idx, size_t num, size_t max_len) {
	(void)buffer; (void)idx;

	for (size_t i = 0; i < num; i++) {
		if (i >= max_len) {
			return false;
		}

		if (in[i]) {
			if (putchar(in[i]) == EOF) {
				return false;
			}
		}
	}
	return true;
}


int printf(const char* restrict format, ...) {
	va_list parameters;
	va_start(parameters, format);
 
	char buffer[1];

	int written = _vsnprintf(_print, buffer, (size_t)-1, format, parameters);
 
	va_end(parameters);
	return written;
}