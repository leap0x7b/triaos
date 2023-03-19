#ifndef LIB__E9_H
#define LIB__E9_H

#include <stdarg.h>

void e9_write_char(char c);
void e9_write(const char *string);
int e9_printf(const char *format, ...);
int e9_vprintf(const char *format, va_list args);

#endif
