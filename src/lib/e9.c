#include <stddef.h>
#include <lib/e9.h>
#include <lib/string.h>
#include <lib/io.h>

void e9_write_char(char c) {
    outb(0xe9, c);
}

void e9_write(const char *string) {
    for (size_t i = 0; i < strlen(string); i++)
        e9_write_char(string[i]);
}
