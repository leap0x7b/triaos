#ifndef LIB__CONSOLE_H
#define LIB__CONSOLE_H

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

typedef enum {
    CONSOLE_COLOR_BLACK = 0,
    CONSOLE_COLOR_BLUE = 1,
    CONSOLE_COLOR_GREEN = 2,
    CONSOLE_COLOR_CYAN = 3,
    CONSOLE_COLOR_RED = 4,
    CONSOLE_COLOR_MAGENTA = 5,
    CONSOLE_COLOR_BROWN = 6,
    CONSOLE_COLOR_LIGHT_GREY = 7,
    CONSOLE_COLOR_DARK_GREY = 8,
    CONSOLE_COLOR_LIGHT_BLUE = 9,
    CONSOLE_COLOR_LIGHT_GREEN = 10,
    CONSOLE_COLOR_LIGHT_CYAN = 11,
    CONSOLE_COLOR_LIGHT_RED = 12,
    CONSOLE_COLOR_LIGHT_MAGENTA = 13,
    CONSOLE_COLOR_LIGHT_BROWN = 14,
    CONSOLE_COLOR_WHITE = 15,
} TiConsoleColor;
 
void TiConsoleInit(void);
void TiConsoleSetCursor(size_t x, size_t y);
uint16_t TiConsoleGetCursor(void);
void TiConsoleSetFont(const uint8_t *buf);
void TiConsoleGetFont(uint8_t *buf);
void TiConsoleSetColor(TiConsoleColor fg, TiConsoleColor bg);
void TiConsoleClear(void);
void TiConsoleWriteCharAt(char c, uint8_t color, size_t column, size_t row);
void TiConsoleWriteChar(char c);
void TiConsoleWrite(const char *string);
int TiConsolePrintf(const char *format, ...);
int TiConsoleVPrintf(const char *format, va_list args);

#endif
