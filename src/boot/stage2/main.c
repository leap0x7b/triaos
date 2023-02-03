#include <lib/vga.h>
#include <lib/e9.h>

void _start(void) {
    vga_init();
    vga_write("bo");
}
