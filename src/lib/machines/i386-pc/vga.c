#include <stdint.h>
#include <lib/string.h>
#include <lib/nanoprintf_config.h>
#include <lib/nanoprintf.h>
#include <lib/i386-pc/vga.h>
#include <lib/i386-pc/io.h>
 
#define VGA_COLUMNS 80
#define VGA_ROWS 25
#define VGA_ADDRESS 0xB8000

static size_t vga_row;
static size_t vga_column;
static uint8_t vga_color;
static uint16_t *vga_buffer = (uint16_t *)VGA_ADDRESS;

void TiVgaSetCursor(size_t offset) {
    TiIoOutByte(0x3D4, 14);
    TiIoOutByte(0x3D5, (uint8_t)(offset >> 8));
    TiIoOutByte(0x3D4, 15);
    TiIoOutByte(0x3D5, (uint8_t)(offset & 0xff));
}

size_t TiVgaGetCursor(void) {
    TiIoOutByte(0x3D4, 14);
    size_t offset = TiIoInByte(0x3D5) << 8;
    TiIoOutByte(0x3D4, 15);
    offset += TiIoInByte(0x3D5);
    return offset;
}

void TiVgaInit(void) {
    vga_row = TiVgaGetCursor() / VGA_COLUMNS;
    vga_column = TiVgaGetCursor() % VGA_COLUMNS;
    vga_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
}

void VgaSetColor(TiVgaColor fg, TiVgaColor bg) {
    vga_color = vga_entry_color(fg, bg);
}

void TiVgaWriteCharAt(char c, uint8_t color, size_t column, size_t row) {
    vga_buffer[row * VGA_COLUMNS + column] = vga_entry(c, color);
}

void TiVgaWriteChar(char c) {
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
            TiVgaWriteCharAt(c, vga_color, vga_column, vga_row);
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

    TiVgaSetCursor(vga_row * VGA_COLUMNS + vga_column);
}

void TiVgaWrite(const char *string) {
    for (size_t i = 0; i < strlen(string); i++)
        TiVgaWriteChar(string[i]);
}

static void _printf_callback(int c, void *_) {
    (void)_;
    TiVgaWriteChar(c);
}

int TiVgaPrintf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    int ret = npf_vpprintf(&_printf_callback, NULL, format, args);
    va_end(args);
    return ret;
}

int TiVgaVPrintf(const char *format, va_list args) {
    return npf_vpprintf(&_printf_callback, NULL, format, args);
}
