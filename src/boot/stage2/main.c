#include <lib/vga.h>
#include <lib/e9.h>
#include <lib/printf.h>
#include <boot/e820.h>
#include <boot/a20.h>

void entry(void) {
    if (a20_init()) {
        e9_write("[triaboot-stage2] Hello World!\n");
        vga_init();
        vga_write("bo");
        e9_write("[triaboot-stage2] E820 memory map: ");
        e820_init();
        e9_write("OK\n");
        //printf_("what");
    } else {
        e9_write("[triaboot-stage2] Unable to boot without A20 gate!\n");
        vga_write("FFC0");
    }
}
