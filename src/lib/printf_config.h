#include <lib/e9.h>

#define PRINTF_SUPPORT_DECIMAL_SPECIFIERS 0
#define PRINTF_SUPPORT_EXPONENTIAL_SPECIFIERS 0
#define PRINTF_SUPPORT_LONG_LONG 0

void putchar_(const char c) {
    e9_write_char(c);
}
