#include <lib/vga.h>
#include <lib/e9.h>
#include <boot/e820.h>

void entry(void) {
    e9_write("[triaboot-stage2] Hello World!\n");
    vga_init();
    vga_write("bo");
    e9_write("[triaboot-stage2] E820 memory map: ");
    e820_init();
    e9_write("OK\n");
}
