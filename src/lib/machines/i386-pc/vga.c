#include <stdint.h>
#include <lib/string.h>
#include <lib/nanoprintf_config.h>
#include <lib/nanoprintf.h>
#include <lib/i386-pc/vga.h>
#include <lib/i386-pc/io.h>
 
#define VGA_COLUMNS 80
#define VGA_ROWS 25
#define VGA_SIZE ((VGA_COLUMNS * VGA_ROWS) * sizeof(uint16_t))
#define VGA_ADDRESS 0xB8000

static size_t vga_row;
static size_t vga_column;
static uint8_t vga_color;
static volatile uint16_t *vga_buffer = (volatile uint16_t *)VGA_ADDRESS;

static inline uint8_t TiVgaEntryColor(TiVgaColor fg, TiVgaColor bg) {
    return (fg | (bg << 4));
}

static inline uint16_t TiVgaEntry(char c, uint8_t color) {
    return c | ((uint16_t)color << 8);
}

void TiVgaInit(void) {
    vga_row = TiVgaGetCursor() / VGA_COLUMNS;
    vga_column = TiVgaGetCursor() % VGA_COLUMNS;
    vga_color = TiVgaEntryColor(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
}

void TiVgaSetCursor(size_t x, size_t y) {
    uint16_t offset = y * VGA_COLUMNS + x;

    TiIoOutByte(0x3D4, 0x0F);
    TiIoOutByte(0x3D5, (uint8_t)(offset) & 0xFF);

    TiIoOutByte(0x3D4, 0x0E);
    TiIoOutByte(0x3D5, (uint8_t)(offset >> 8) & 0xFF);
}

uint16_t TiVgaGetCursor(void) {
    uint16_t offset = 0;

    TiIoOutByte(0x3D4, 0x0F);
    offset |= TiIoInByte(0x3D5);

    TiIoOutByte(0x3D4, 0x0E);
    offset |= (uint16_t)(TiIoInByte(0x3D5)) << 8;

    return offset;
}

void TiVgaSetColor(TiVgaColor fg, TiVgaColor bg) {
    vga_color = TiVgaEntryColor(fg, bg);
}

void TiVgaClear(void) {
    memset((void *)vga_buffer, TiVgaEntry(' ', vga_color), VGA_SIZE);
}

void TiVgaWriteCharAt(char c, uint8_t color, size_t column, size_t row) {
    vga_buffer[row * VGA_COLUMNS + column] = TiVgaEntry(c, color);
}

static void scroll(void) {
    size_t x = 0;
    size_t y = 0;

    for (y = 0; y < VGA_ROWS - 1; y++) {
        for (x = 0; x < VGA_COLUMNS; x++) {
            vga_buffer[y * VGA_COLUMNS + x] = vga_buffer[(y + 1) * VGA_COLUMNS + x];
        }
    }

    for (x = 0; x < VGA_COLUMNS; x++) {
        vga_buffer[y * VGA_COLUMNS + x] = TiVgaEntry(' ', vga_color);
    }

    vga_column = 0;
    vga_row = VGA_ROWS - 1;
}

void TiVgaWriteChar(char c) {
    switch (c) {
        case '\r':
            vga_column = 0;
            break;

        case '\n':
            vga_row++;
            vga_column = 0;
            break;

        case '\t':
            // TODO: implement actual tabs instead of simply +=ing
            //       vga_column
            vga_column += 8;
            break;

        default:
            TiVgaWriteCharAt(c, vga_color, vga_column, vga_row);
            break;
    }

    if (c != '\n') {
        if (++vga_column == VGA_COLUMNS) {
            vga_column = 0;
            if (++vga_row == VGA_ROWS) {
                scroll();
            }
        }
    } else {
        if (vga_row == VGA_ROWS) {
            scroll();
        }
    }

    TiVgaSetCursor(vga_column, vga_row);
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
