#include <stdint.h>
#include <lib/printf.h>
#include <lib/nanoprintf_config.h>
#include <lib/nanoprintf.h>

int snprintf(char* buffer, size_t count, const char* format, ...) {
    va_list args;
    va_start(args, format);
    int ret = npf_vsnprintf(buffer, count, format, args);
    va_end(args);
    return ret;
}

int vsnprintf(char* buffer, size_t count, const char* format, va_list args) {
    return npf_vsnprintf(buffer, count, format, args);
}
