#ifndef LIB__E9_H
#define LIB__E9_H

#include <stdarg.h>

void TiE9Char(char c);
void TiE9(const char *string);
int TiE9Printf(const char *format, ...);
int TiE9VPrintf(const char *format, va_list args);

#endif
