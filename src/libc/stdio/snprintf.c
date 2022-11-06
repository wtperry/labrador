#include <libk/stdio.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdbool.h>
#include "_vsnprintf.h"

int _out_s(const char* in, void* buffer, size_t idx, size_t num, size_t max_len) {
    char* out = (char*)buffer;

	for (size_t i = 0; i < num; i++) {
		if (i + idx >= max_len) {
			return false;
		}

		out[i + idx] = in[i];
	}
	return true;
}

int snprintf(char* restrict str, size_t size, const char* restrict format, ...) {
    va_list parameters;
	va_start(parameters, format);

	int written = _vsnprintf(_out_s, str, size, format, parameters);
 
	va_end(parameters);
	return written;
}

int vsnprintf(char* restrict str, size_t size, const char* restrict format, va_list args) {
	return _vsnprintf(_out_s, str, size, format, args);
}