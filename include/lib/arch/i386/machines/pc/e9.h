#ifndef LIB__I386_PC__E9_H
#define LIB__I386_PC__E9_H

#include <stdarg.h>

void TiE9WriteChar(char c);
void TiE9Write(const char *string);
int TiE9Printf(const char *format, ...);
int TiE9VPrintf(const char *format, va_list args);

#endif
