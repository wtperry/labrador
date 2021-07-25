#pragma once

#include <stddef.h>
#include <stdint.h>
#include <bootloader/psf1.h>

void terminal_initialize(void* fb_ptr, size_t width, size_t height, size_t scanline, PSF1_FONT* Font);
void terminal_clear(void);
void terminal_putchar(char c);
void terminal_write(const char* data, size_t size);
void terminal_writestring(const char* data);