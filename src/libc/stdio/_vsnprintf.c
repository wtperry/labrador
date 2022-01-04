#include "_vsnprintf.h"
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stddef.h>
#include <stdbool.h>

// internal flag definitions
#define FLAGS_ZEROPAD   (1U <<  0U)
#define FLAGS_LEFT      (1U <<  1U)
#define FLAGS_PLUS      (1U <<  2U)
#define FLAGS_SPACE     (1U <<  3U)
#define FLAGS_HASH      (1U <<  4U)
#define FLAGS_UPPERCASE (1U <<  5U)
#define FLAGS_CHAR      (1U <<  6U)
#define FLAGS_SHORT     (1U <<  7U)
#define FLAGS_LONG      (1U <<  8U)
#define FLAGS_LONG_LONG (1U <<  9U)
#define FLAGS_PRECISION (1U << 10U)
#define FLAGS_ADAPT_EXP (1U << 11U)

#define PRINTF_NTOA_BUFFER_SIZE 256

static void _strrev(char* str, size_t len) {
	char swap;

	for (size_t i = 0; i < len/2; i++) {
		swap = str[i];
		str[i] = str[len - i - 1];
		str[len - i - 1] = swap;
	}
}

// internal itoa
static size_t _ntoa(unsigned long long value, char* str_buffer, unsigned int base, bool negative, unsigned int prec, unsigned int width, unsigned int flags) {
	size_t len = 0;
	
	// no hash for 0 values
	if (!value) {
		flags &= ~FLAGS_HASH;
	}

	if (!(flags & FLAGS_PRECISION) || value) {
		do {
			const char digit = (char) (value % base);
			if (digit < 10) {
				str_buffer[len++] = '0' + digit;
			} else {
				str_buffer[len++] = 'a' + digit - 10;
			}

			value /= base;
		} while (value && len < PRINTF_NTOA_BUFFER_SIZE);
	}

	// pad leading zeros and spaces
	if (!(flags & FLAGS_LEFT)) {
		if (width && (flags & FLAGS_ZEROPAD) && (negative || (flags & (FLAGS_PLUS | FLAGS_SPACE)))) {
			width--;
		}
		while ((len < prec) && (len < PRINTF_NTOA_BUFFER_SIZE)) {
			str_buffer[len++] = '0';
		}
		while ((flags & FLAGS_ZEROPAD) && (len < width) && (len < PRINTF_NTOA_BUFFER_SIZE)) {
			str_buffer[len++] = '0';
		}
		while ((len < width) && (len < PRINTF_NTOA_BUFFER_SIZE)) {
			str_buffer[len++] = ' ';
		}
	}

	// handle hash
	if (flags & FLAGS_HASH) {
		if (!(flags & FLAGS_PRECISION) && len && ((len == prec) || (len == width))) {
			len--;
			if (len && (base == 16)) {
				len--;
			}
		}
		if ((base == 16) && !(flags & FLAGS_UPPERCASE) && (len < PRINTF_NTOA_BUFFER_SIZE)) {
			str_buffer[len++] = 'x';
		} else if ((base == 16) && (flags & FLAGS_UPPERCASE) && (len < PRINTF_NTOA_BUFFER_SIZE)) {
			str_buffer[len++] = 'X';
		}
		if (len < PRINTF_NTOA_BUFFER_SIZE) {
			str_buffer[len++] = '0';
		}
	}

	// handle signs
	if (len < PRINTF_NTOA_BUFFER_SIZE) {
		if (negative) {
			str_buffer[len++] = '-';
		} else if (flags & FLAGS_PLUS) {
			str_buffer[len++] = '+';
		} else if (flags & FLAGS_SPACE) {
			str_buffer[len++] = ' ';
		}
	}

	_strrev(str_buffer, len);

	// add spaces for left justify
	if (flags & FLAGS_LEFT) {
		while ((len < width) && (len < PRINTF_NTOA_BUFFER_SIZE)) {
			str_buffer[len++] = ' ';
		}
	}

	return len;
}
 
int _vsnprintf(print_fct_t print, char* restrict buffer, const size_t max_len, const char* restrict format, va_list va) {
	unsigned int flags, width, precision, n;
	char str_buffer[PRINTF_NTOA_BUFFER_SIZE];

	size_t written = 0;
	size_t i = 0;
 
	while (format[i] != '\0') {
		size_t maxrem = max_len - written;
 
		if (format[i] != '%' || format[i+1] == '%') {
			if (format[i] == '%')
				i++;
			size_t amount = 1;
			while (format[i + amount] && format[i + amount] != '%')
				amount++;
			if (maxrem < amount) {
				// TODO: Set errno to EOVERFLOW.
				return -1;
			}
			if (!print(format + i, buffer, written, amount, max_len))
				return -1;
			i += amount;
			written += amount;
			continue;
		}
 
		const size_t format_begun_at = i;
		i++;
 
		// evaluate flags
		flags = 0;
		do {
			switch (format[i]) {
				case '0':	flags |= FLAGS_ZEROPAD;	i++;	n = 1;	break;
				case '-':	flags |= FLAGS_LEFT;	i++;	n = 1;	break;
				case '+':	flags |= FLAGS_PLUS;	i++;	n = 1;	break;
				case ' ':	flags |= FLAGS_SPACE;	i++;	n = 1;	break;
				case '#':	flags |= FLAGS_HASH;	i++;	n = 1;	break;
				default:									n = 0;	break;
			}
		} while (n);

		// evaluate width field
		width = 0;
		if (isdigit(format[i])) {
			width = atoi(format + i);
			while(isdigit(format[i])) {
				i++;
			}
		} else if (format[i] == '*') {
			const int w = va_arg(va, int);
			if (w < 0) {
				flags |= FLAGS_LEFT;
				width = (unsigned int)(-w);
			} else {
				width = (unsigned int)w;
			}
			i++;
		}

		// evaluate precision field
		precision = 0;
		if (format[i] == '.') {
			flags |= FLAGS_PRECISION;
			i++;
			if (isdigit(format[i])) {
				precision = atoi(format + i);
				while(isdigit(format[i])) {
					i++;
				}
			} else if (format[i] == '*') {
				const int prec = (int)va_arg(va, int);
				precision = prec > 0 ? (unsigned int)prec : 0;
				format++;
			}
		}

		// evaluate length field
		switch (format[i]) {
			case 'l' :
			flags |= FLAGS_LONG;
			i++;
			if (format[i] == 'l') {
				flags |= FLAGS_LONG_LONG;
				i++;
			}
			break;
			
			case 'h' :
			flags |= FLAGS_SHORT;
			i++;
			if (format[i] == 'h') {
				flags |= FLAGS_CHAR;
				i++;
			}
			break;
		}

		switch (format[i]) {
			case 'd':
			case 'i':
			case 'u':
			case 'x':
			case 'X':
			case 'o':
			case 'p': {
				// set the base
				unsigned int base;
				if (format[i] == 'x' || format[i] == 'X' || format[i] == 'p') {
					base = 16;
				} else if (format[i] == 'o') {
					base = 8;
				} else {
					base = 10;
					flags &= ~FLAGS_HASH;	// no hash for decimal
				}

				if (format[i] == 'X') {
					flags |= FLAGS_UPPERCASE;
				}

				// no plus or space flag for um x, X, o
				if ((format[i] != 'i') && (format[i] != 'd')) {
					flags &= ~(FLAGS_PLUS | FLAGS_SPACE);
				}

				// ignore '0' flag when precision is given
				if (flags & FLAGS_PRECISION) {
					flags &= ~FLAGS_ZEROPAD;
				}

				size_t len = 0; // output string length

				if ((format[i] == 'i') || (format[i] == 'd')) {
					// signed
					if (flags & FLAGS_LONG_LONG) {
						const long long value = (long long) va_arg(va, long long);
						len = _ntoa(value, str_buffer, base, value < 0, precision, width, flags);
					} else if (flags & FLAGS_LONG) {
						const long value = va_arg(va, long);
						len = _ntoa(value, str_buffer, base, value < 0, precision, width, flags);
					} else {
						const int value = (flags & FLAGS_CHAR) ? (char)va_arg(va, int) : (flags & FLAGS_SHORT) ? (short int)va_arg(va, int) : va_arg(va, int);
						len = _ntoa(value, str_buffer, base, value < 0, precision, width, flags);
					}
				} else if (format[i] == 'p') {
					//pointer
					const unsigned long long value = (unsigned long long)va_arg(va, void*);
					len = _ntoa(value, str_buffer, base, false, precision, width, flags);
				} else {
					// unsigned
					if (flags & FLAGS_LONG_LONG) {
						len = _ntoa(va_arg(va, unsigned long long), str_buffer, base, false, precision, width, flags);
					} else if (flags & FLAGS_LONG) {
						len = _ntoa(va_arg(va, unsigned long), str_buffer, base, false, precision, width, flags);
					} else {
						const unsigned int value = (flags & FLAGS_CHAR) ? (unsigned char)va_arg(va, unsigned int) : (flags & FLAGS_SHORT) ? (unsigned short int)va_arg(va, unsigned int) : va_arg(va, unsigned int);
						len = _ntoa(value, str_buffer, base, false, precision, width, flags);
					}
				}

				if (!print(str_buffer, buffer, written, len, max_len))
					return -1;

				written += len;
				i++;
				break;
			}

			case 'c': {
				size_t len = 1;
				// left padding
				if (!(flags & FLAGS_LEFT)) {
					while (len < width) {
						str_buffer[len-1] = ' ';
						len++;
					}
				}

				str_buffer[len-1] = (char) va_arg(va, int);

				// right padding
				if (flags & FLAGS_LEFT) {
					while (len < width) {
						str_buffer[len] = ' ';
						len++;
					}
				}

				if (!print(str_buffer, buffer, written, len, max_len))
					return -1;

				written += len;
				i++;
				break;
			}

			case 's': {
				const char* str = va_arg(va, const char*);
				size_t str_len = strlen(str);

				if ((flags & FLAGS_PRECISION) && str_len > precision) {
					str_len = precision;
				}

				size_t len = 0;

				if (!(flags & FLAGS_LEFT)) {
					while ((len + str_len) < width) {
						str_buffer[len] = ' ';
						len++;
					}
				}

				memcpy(str_buffer + len, str, str_len);
				len += str_len;

				if (flags & FLAGS_LEFT) {
					while (len < width) {
						str_buffer[len] = ' ';
						len++;
					}
				}

				if (!print(str_buffer, buffer, written, len, max_len))
					return -1;

				written += len;
				i++;
				break;
			}

			default: {
				if (!print(format + format_begun_at, buffer, written, 1, max_len))
					return -1;
				
				i = format_begun_at + 1;
				written++;
				break;
			}
		}
	}
 
	//null terminate string
	str_buffer[0] = '\0';
	print(str_buffer, buffer, written < max_len ? written : max_len - 1, 1, max_len);
	
	return written;
}