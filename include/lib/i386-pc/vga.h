#ifndef LIB__VGA_H
#define LIB__VGA_H

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

typedef enum {
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
} TiVgaColor;
 
void TiVgaInit(void);
void TiVgaSetCursor(size_t x, size_t y);
uint16_t TiVgaGetCursor(void);
void VgaSetColor(TiVgaColor fg, TiVgaColor bg);
void TiVgaClear(void);
void TiVgaWriteCharAt(char c, uint8_t color, size_t column, size_t row);
void TiVgaWriteChar(char c);
void TiVgaWrite(const char *string);
int TiVgaPrintf(const char *format, ...);
int TiVgaVPrintf(const char *format, va_list args);

#endif
