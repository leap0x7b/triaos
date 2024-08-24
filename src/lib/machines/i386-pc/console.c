#include <stdint.h>
#include <lib/string.h>
#include <lib/nanoprintf_config.h>
#include <lib/nanoprintf.h>
#include <lib/i386-pc/console.h>
#include <lib/i386-pc/io.h>
#include <lib/i386-pc/vga.h>
 
#define CONSOLE_COLUMNS 80
#define CONSOLE_ROWS 25
#define CONSOLE_SIZE ((CONSOLE_COLUMNS * CONSOLE_ROWS) * sizeof(uint16_t))
#define CONSOLE_ADDRESS 0xB8000

static size_t console_row;
static size_t console_column;
static uint8_t console_color;
static volatile uint16_t *console_buffer = (volatile uint16_t *)CONSOLE_ADDRESS;

static inline uint8_t TiConsoleEntryColor(TiConsoleColor fg, TiConsoleColor bg) {
    return (fg | (bg << 4));
}

static inline uint16_t TiConsoleEntry(char c, uint8_t color) {
    return c | ((uint16_t)color << 8);
}

void TiConsoleInit(void) {
    console_column = TiConsoleGetCursor() % CONSOLE_COLUMNS;
    console_row = TiConsoleGetCursor() / CONSOLE_COLUMNS;
    console_color = TiConsoleEntryColor(CONSOLE_COLOR_LIGHT_GREY, CONSOLE_COLOR_BLACK);
}

void TiConsoleSetCursor(size_t x, size_t y) {
    uint16_t offset = y * CONSOLE_COLUMNS + x;

    TiIoOutByte(0x3D4, 0x0F);
    TiIoOutByte(0x3D5, (uint8_t)(offset) & 0xFF);

    TiIoOutByte(0x3D4, 0x0E);
    TiIoOutByte(0x3D5, (uint8_t)(offset >> 8) & 0xFF);
}

uint16_t TiConsoleGetCursor(void) {
    uint16_t offset = 0;

    TiIoOutByte(0x3D4, 0x0F);
    offset |= TiIoInByte(0x3D5);

    TiIoOutByte(0x3D4, 0x0E);
    offset |= (uint16_t)(TiIoInByte(0x3D5)) << 8;

    return offset;
}

void TiConsoleSetFont(const uint8_t *buf) {
    volatile uint8_t *mem = (volatile uint8_t *)0xA0000;

    // Clear even/odd mode
    TiIoOutShort(VGA_GC_INDEX, 5);
    // Map VGA memory to 0xA0000
    TiIoOutShort(VGA_GC_INDEX, 0x406);
    // Set bitplane 2
    TiIoOutShort(VGA_SEQ_INDEX, 0x402);
    // Clear even/odd mode (the other way)
    TiIoOutShort(VGA_SEQ_INDEX, 0x604);

    // Copy charmap
    for (int i = 0; i < 256; ++i) {
        for (int j = 0; j < 16; ++j) {
            *mem++ = *buf++;
        }
        mem += 16; // Skip the next 16 bytes in VGA memory
    }

    // Restore VGA state to normal operation
    TiIoOutShort(VGA_SEQ_INDEX, 0x302);
    TiIoOutShort(VGA_SEQ_INDEX, 0x204);
    TiIoOutShort(VGA_GC_INDEX, 0x1005);
    TiIoOutShort(VGA_GC_INDEX, 0xE00);
}

void TiConsoleGetFont(uint8_t *buf) {
    volatile uint8_t *mem = (volatile uint8_t *)0xA0000;

    // Clear even/odd mode
    TiIoOutShort(VGA_GC_INDEX, 5);
    // Map VGA memory to 0xA0000
    TiIoOutShort(VGA_GC_INDEX, 0x406);
    // Set bitplane 2
    TiIoOutShort(VGA_SEQ_INDEX, 0x402);
    // Clear even/odd mode (the other way)
    TiIoOutShort(VGA_SEQ_INDEX, 0x604);

    // Copy charmap
    for (int i = 0; i < 256; ++i) {
        for (int j = 0; j < 16; ++j) {
            *buf++ = *mem++;
        }
        mem += 16; // Skip the next 16 bytes in VGA memory
    }

    // Restore VGA state to normal operation
    TiIoOutShort(VGA_SEQ_INDEX, 0x302);
    TiIoOutShort(VGA_SEQ_INDEX, 0x204);
    TiIoOutShort(VGA_GC_INDEX, 0x1005);
    TiIoOutShort(VGA_GC_INDEX, 0xE00);
}

void TiConsoleSetColor(TiConsoleColor fg, TiConsoleColor bg) {
    console_color = TiConsoleEntryColor(fg, bg);
}

void TiConsoleClear(void) {
    memset((void *)console_buffer, TiConsoleEntry(' ', console_color), CONSOLE_SIZE);
}

void TiConsoleWriteCharAt(char c, uint8_t color, size_t column, size_t row) {
    console_buffer[row * CONSOLE_COLUMNS + column] = TiConsoleEntry(c, color);
}

static void scroll(void) {
    size_t x = 0;
    size_t y = 0;

    for (y = 0; y < CONSOLE_ROWS - 1; y++) {
        for (x = 0; x < CONSOLE_COLUMNS; x++) {
            console_buffer[y * CONSOLE_COLUMNS + x] = console_buffer[(y + 1) * CONSOLE_COLUMNS + x];
        }
    }

    for (x = 0; x < CONSOLE_COLUMNS; x++) {
        console_buffer[y * CONSOLE_COLUMNS + x] = TiConsoleEntry(' ', console_color);
    }

    console_column = 0;
    console_row = CONSOLE_ROWS - 1;
}

void TiConsoleWriteChar(char c) {
    switch (c) {
        case '\r':
            console_column = 0;
            break;

        case '\n':
            console_row++;
            console_column = 0;
            break;

        case '\t':
            // TODO: implement actual tabs instead of simply +=ing
            //       console_column
            console_column += 8;
            break;

        default:
            TiConsoleWriteCharAt(c, console_color, console_column, console_row);
            break;
    }

    if (c != '\n') {
        if (++console_column == CONSOLE_COLUMNS) {
            console_column = 0;
            if (++console_row == CONSOLE_ROWS) {
                scroll();
            }
        }
    } else {
        if (console_row == CONSOLE_ROWS) {
            scroll();
        }
    }

    TiConsoleSetCursor(console_column, console_row);
}

void TiConsoleWrite(const char *string) {
    for (size_t i = 0; i < strlen(string); i++)
        TiConsoleWriteChar(string[i]);
}

static void _printf_callback(int c, void *_) {
    (void)_;
    TiConsoleWriteChar(c);
}

int TiConsolePrintf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    int ret = npf_vpprintf(&_printf_callback, NULL, format, args);
    va_end(args);
    return ret;
}

int TiConsoleVPrintf(const char *format, va_list args) {
    return npf_vpprintf(&_printf_callback, NULL, format, args);
}
