#ifndef LIB__PRINTF_H
#define LIB__PRINTF_H

#include <stdarg.h>
#include <stddef.h>

int snprintf(char* buffer, size_t count, const char* format, ...);
int vsnprintf(char* buffer, size_t count, const char* format, va_list args);

#endif
