#include <lib/i386-pc/vga.h>
#include <lib/i386-pc/e9.h>

void Main(void) {
    TiE9Write("[triaboot-stage2] Hello World!\n");
    TiVgaInit();
    TiVgaWriteChar('r');
}
