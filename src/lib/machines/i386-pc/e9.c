#include <stddef.h>
#include <lib/e9.h>
#include <lib/string.h>
#include <lib/io.h>
#include <lib/nanoprintf_config.h>
#include <lib/nanoprintf.h>

void TiE9Char(char c) {
    TiIoOutByte(0xE9, c);
}

void TiE9(const char *string) {
    for (size_t i = 0; i < strlen(string); i++)
        TiE9Char(string[i]);
}

static void _printf_callback(int c, void *_) {
    (void)_;
    TiE9Char(c);
}

int TiE9Printf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    int ret = npf_vpprintf(&_printf_callback, NULL, format, args);
    va_end(args);
    return ret;
}

int TiE9VPrintf(const char *format, va_list args) {
    return npf_vpprintf(&_printf_callback, NULL, format, args);
}
