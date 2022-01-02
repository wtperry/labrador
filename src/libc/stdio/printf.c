#include <limits.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

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

static bool print(const char* data, size_t length) {
	const unsigned char* bytes = (const unsigned char*) data;
	for (size_t i = 0; i < length; i++)
		if (putchar(bytes[i]) == EOF)
			return false;
	return true;
}

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
 
int printf(const char* restrict format, ...) {
	va_list parameters;
	va_start(parameters, format);
 
	unsigned int flags, width, precision, n;
	char str_buffer[PRINTF_NTOA_BUFFER_SIZE];

	int written = 0;
 
	while (*format != '\0') {
		size_t maxrem = INT_MAX - written;
 
		if (format[0] != '%' || format[1] == '%') {
			if (format[0] == '%')
				format++;
			size_t amount = 1;
			while (format[amount] && format[amount] != '%')
				amount++;
			if (maxrem < amount) {
				// TODO: Set errno to EOVERFLOW.
				return -1;
			}
			if (!print(format, amount))
				return -1;
			format += amount;
			written += amount;
			continue;
		}
 
		const char* format_begun_at = format++;
 
		// evaluate flags
		flags = 0;
		do {
			switch (*format) {
				case '0':	flags |= FLAGS_ZEROPAD;	format++;	n = 1;	break;
				case '-':	flags |= FLAGS_LEFT;	format++;	n = 1;	break;
				case '+':	flags |= FLAGS_PLUS;	format++;	n = 1;	break;
				case ' ':	flags |= FLAGS_SPACE;	format++;	n = 1;	break;
				case '#':	flags |= FLAGS_HASH;	format++;	n = 1;	break;
				default:										n = 0;	break;
			}
		} while (n);

		// evaluate width field
		width = 0;
		if (isdigit(*format)) {
			width = atoi(format);
			while(isdigit(*format)) {
				format++;
			}
		} else if (*format == '*') {
			const int w = va_arg(parameters, int);
			if (w < 0) {
				flags |= FLAGS_LEFT;
				width = (unsigned int)(-w);
			} else {
				width = (unsigned int)w;
			}
			format++;
		}

		// evaluate precision field
		precision = 0;
		if (*format == '.') {
			flags |= FLAGS_PRECISION;
			format++;
			if (isdigit(*format)) {
				precision = atoi(format);
				while(isdigit(*format)) {
					format++;
				}
			} else if (*format == '*') {
				const int prec = (int)va_arg(parameters, int);
				precision = prec > 0 ? (unsigned int)prec : 0;
				format++;
			}
		}

		// evaluate length field
		switch (*format) {
			case 'l' :
			flags |= FLAGS_LONG;
			format++;
			if (*format == 'l') {
				flags |= FLAGS_LONG_LONG;
				format++;
			}
			break;
			
			case 'h' :
			flags |= FLAGS_SHORT;
			format++;
			if (*format == 'h') {
				flags |= FLAGS_CHAR;
				format++;
			}
			break;
		}

		switch (*format) {
			case 'd':
			case 'i':
			case 'u':
			case 'x':
			case 'X':
			case 'o':
			case 'p': {
				// set the base
				unsigned int base;
				if (*format == 'x' || *format == 'X' || *format == 'p') {
					base = 16;
				} else if (*format == 'o') {
					base = 8;
				} else {
					base = 10;
					flags &= ~FLAGS_HASH;	// no hash for decimal
				}

				if (*format == 'X') {
					flags |= FLAGS_UPPERCASE;
				}

				// no plus or space flag for um x, X, o
				if ((*format != 'i') && (*format != 'd')) {
					flags &= ~(FLAGS_PLUS | FLAGS_SPACE);
				}

				// ignore '0' flag when precision is given
				if (flags & FLAGS_PRECISION) {
					flags &= ~FLAGS_ZEROPAD;
				}

				size_t len = 0; // output string length

				if ((*format == 'i') || (*format == 'd')) {
					// signed
					if (flags & FLAGS_LONG_LONG) {
						const long long value = (long long) va_arg(parameters, long long);
						len = _ntoa(value, str_buffer, base, value < 0, precision, width, flags);
					} else if (flags & FLAGS_LONG) {
						const long value = va_arg(parameters, long);
						len = _ntoa(value, str_buffer, base, value < 0, precision, width, flags);
					} else {
						const int value = (flags & FLAGS_CHAR) ? (char)va_arg(parameters, int) : (flags & FLAGS_SHORT) ? (short int)va_arg(parameters, int) : va_arg(parameters, int);
						len = _ntoa(value, str_buffer, base, value < 0, precision, width, flags);
					}
				} else if (*format == 'p') {
					//pointer
					const unsigned long long value = (unsigned long long)va_arg(parameters, void*);
					len = _ntoa(value, str_buffer, base, false, precision, width, flags);
				} else {
					// unsigned
					if (flags & FLAGS_LONG_LONG) {
						len = _ntoa(va_arg(parameters, unsigned long long), str_buffer, base, false, precision, width, flags);
					} else if (flags & FLAGS_LONG) {
						len = _ntoa(va_arg(parameters, unsigned long), str_buffer, base, false, precision, width, flags);
					} else {
						const unsigned int value = (flags & FLAGS_CHAR) ? (unsigned char)va_arg(parameters, unsigned int) : (flags & FLAGS_SHORT) ? (unsigned short int)va_arg(parameters, unsigned int) : va_arg(parameters, unsigned int);
						len = _ntoa(value, str_buffer, base, false, precision, width, flags);
					}
				}

				if (!print(str_buffer, len))
					return -1;

				written += len;
				format++;
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

				str_buffer[len-1] = (char) va_arg(parameters, int);

				// right padding
				if (flags & FLAGS_LEFT) {
					while (len < width) {
						str_buffer[len] = ' ';
						len++;
					}
				}

				if (!print(str_buffer, len))
					return -1;

				written += len;
				format++;
				break;
			}

			case 's': {
				const char* str = va_arg(parameters, const char*);
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

				if (!print(str_buffer, len))
					return -1;

				written += len;
				format++;
				break;
			}

			default: {
				if (!print(format_begun_at, 1))
					return -1;
				
				format = format_begun_at + 1;
				written++;
				break;
			}
		}
	}
 
	va_end(parameters);
	return written;
}