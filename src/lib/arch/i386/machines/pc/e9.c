#include <stddef.h>
#include <lib/arch/i386/io.h>
#include <lib/arch/i386/machines/pc/e9.h>
#include <lib/string.h>
#include <lib/nanoprintf_config.h>
#include <lib/nanoprintf.h>

void TiE9WriteChar(char c) {
    TiIoOutByte(0xE9, c);
}

void TiE9Write(const char *string) {
    for (size_t i = 0; i < strlen(string); i++)
        TiE9WriteChar(string[i]);
}

static void _printf_callback(int c, void *_) {
    (void)_;
    TiE9WriteChar(c);
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
