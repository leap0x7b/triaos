#ifndef LIB__VGA_H
#define LIB__VGA_H

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

typedef enum vga_color {
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6,
    VGA_COLOR_LIGHT_GREY = 7,
    VGA_COLOR_DARK_GREY = 8,
    VGA_COLOR_LIGHT_BLUE = 9,
    VGA_COLOR_LIGHT_GREEN = 10,
    VGA_COLOR_LIGHT_CYAN = 11,
    VGA_COLOR_LIGHT_RED = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_LIGHT_BROWN = 14,
    VGA_COLOR_WHITE = 15,
} vga_color_t;

#define vga_entry_color(fg, bg) ((vga_color_t)fg | (vga_color_t)bg << 4)
#define vga_entry(c, color) ((char)c | (uint8_t)color << 8)
 
void vga_init(void);
void vga_set_cursor(size_t offset);
size_t vga_get_cursor(void);
void vga_write_char(char c);
void vga_write(const char *string);
int vga_printf(const char *format, ...);
int vga_vprintf(const char *format, va_list args);

#endif
