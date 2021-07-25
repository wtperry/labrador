#include <kernel/tty.h>

static size_t terminal_hres;
static size_t terminal_vres;

static size_t terminal_height;
static size_t terminal_width;

static size_t terminal_row;
static size_t terminal_column;

static uint32_t terminal_bgcolor;
static uint32_t terminal_color;
static uint32_t* terminal_buffer;

static uint16_t pixels_per_scan_line;

static PSF1_FONT* font;

void terminal_scroll() {
    for (size_t y = 0; y < (terminal_height-1) * font->psf1_header->charsize; y++) {
        for (size_t x = 0; x < terminal_width * 8; x++) {
            *(terminal_buffer + y * pixels_per_scan_line + x) = *(terminal_buffer + (y + font->psf1_header->charsize) * pixels_per_scan_line + x);
        }
    }

    for (size_t y = (terminal_height-1) * font->psf1_header->charsize; y < terminal_height * font->psf1_header->charsize; y++) {
        for (size_t x = 0; x < terminal_width * 8; x++) {
            *(terminal_buffer + y * pixels_per_scan_line + x) = terminal_bgcolor;
        }
    }
}

void terminal_newline() {
    terminal_column = 0;
    if (++terminal_row == terminal_height) {
        terminal_scroll();
        terminal_row--;
    }
}

void terminal_initialize(void* fb_ptr, size_t width, size_t height, size_t scanline, PSF1_FONT* Font) {
    font = Font;
    terminal_buffer = fb_ptr;
    terminal_hres = width;
    terminal_vres = height;
    pixels_per_scan_line = scanline;

    terminal_width = terminal_hres / 8;
    terminal_height = terminal_vres / font->psf1_header->charsize;

    terminal_row = 0;
    terminal_column = 0;

    terminal_color = 0xffffffff;
    terminal_bgcolor = 0x00000000;

    terminal_clear();
}

void terminal_clear() {
    for (size_t y = 0; y < terminal_vres; y++) {
        for (size_t x = 0; x < terminal_hres; x++) {
            uint32_t* pixel = terminal_buffer + (pixels_per_scan_line * y) + x;
            *pixel = terminal_bgcolor;
        }
    }

    terminal_row = 0;
    terminal_column = 0;
}

void terminal_putchar(char c) {
    if (c == '\n') {
        terminal_newline();
    } else {
        size_t x = terminal_column * 8;
        size_t y = terminal_row * font->psf1_header->charsize;
        char* font_ptr = (char*)font->glyph_buffer + (c * font->psf1_header->charsize);

        for (size_t j = y; j < y + font->psf1_header->charsize; j++) {
            for (size_t i = x; i < x + 8; i++) {
                if ((*font_ptr & (0x80 >> (i-x)))) {
                    *(terminal_buffer + j * pixels_per_scan_line + i) = terminal_color;
                }
            }
            font_ptr++;
        }

        if (++terminal_column == terminal_width) {
            terminal_newline();
        }
    }
}

void terminal_write(const char* data, size_t size) {
    for (size_t i = 0; i < size; i++)
		terminal_putchar(data[i]);
}

void terminal_writestring(const char* data) {

}