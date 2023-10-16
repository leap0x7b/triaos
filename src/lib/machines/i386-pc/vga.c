#include <stdint.h>
#include <lib/string.h>
#include <lib/io.h>
#include <lib/vga.h>
#include <lib/nanoprintf_config.h>
#include <lib/nanoprintf.h>
 
#define VGA_COLUMNS 80
#define VGA_ROWS 25
#define VGA_ADDRESS 0xb8000

static size_t vga_row;
static size_t vga_column;
static uint8_t vga_color;
static uint16_t *vga_buffer = (uint16_t *)VGA_ADDRESS;

void vga_set_cursor(size_t offset) {
    outb(0x3d4, 14);
    outb(0x3d5, (uint8_t)(offset >> 8));
    outb(0x3d4, 15);
    outb(0x3d5, (uint8_t)(offset & 0xff));
}

size_t vga_get_cursor(void) {
    outb(0x3d4, 14);
    size_t offset = inb(0x3d5) << 8;
    outb(0x3d4, 15);
    offset += inb(0x3d5);
    return offset;
}

void vga_init(void) {
    vga_row = vga_get_cursor() / VGA_COLUMNS;
    vga_column = vga_get_cursor() % VGA_COLUMNS;
    vga_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
}

void vga_set_color(uint8_t color) {
    vga_color = color;
}

void vga_write_char_at(char c, uint8_t color, size_t column, size_t row) {
    vga_buffer[row * VGA_COLUMNS + column] = vga_entry(c, color);
}

void vga_write_char(char c) {
    switch (c) {
        case '\n':
            vga_row++;
            vga_column = 0;
            break;

        case '\t':
            // TODO: implement actual tabs instead of simply +=ing
            //       vga_column
            vga_column += 4;

        default:
            vga_write_char_at(c, vga_color, vga_column, vga_row);
            vga_column++;
    }

    if (vga_column >= VGA_COLUMNS) {
        vga_column = 0;
        vga_row++;
    }

    if (vga_row >= VGA_ROWS) {
        memmove(vga_buffer, vga_buffer + VGA_COLUMNS, VGA_COLUMNS * (VGA_ROWS - 1) * sizeof(uint16_t));
        size_t index = (VGA_ROWS - 1) * VGA_COLUMNS;
        for (size_t x = 0; x < VGA_COLUMNS; ++x) {
            vga_buffer[index + x] = vga_entry(' ', vga_color);
        }
    }

    vga_set_cursor(vga_row * VGA_COLUMNS + vga_column);
}

void vga_write(const char *string) {
    for (size_t i = 0; i < strlen(string); i++)
        vga_write_char(string[i]);
}

static void _printf_callback(int c, void *_) {
    (void)_;
    vga_write_char(c);
}

int vga_printf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    int ret = npf_vpprintf(&_printf_callback, NULL, format, args);
    va_end(args);
    return ret;
}

int vga_vprintf(const char *format, va_list args) {
    return npf_vpprintf(&_printf_callback, NULL, format, args);
}
