#include <stdint.h>
#include <lib/string.h>
#include <boot/arch/m68k/machines/mac/rom.h>

void *bss_start;
void *bss_end;

void Main(void) {
    //memset(&bss_start, 0, (int)&bss_end - (int)&bss_start);

    BiQdGrafPort grafport;
    BiRomInitGraf(&grafport);
    memset(grafport.bitmap.address, 0, (grafport.bitmap.bounds.bottom - grafport.bitmap.bounds.top) * (grafport.bitmap.row_bytes & 0x3fff));

    BiRomSysError(0xbeef);

    while (1)
        __asm__ volatile("nop");
}

unsigned int __mulsi3(unsigned int a, unsigned int b) {
    unsigned int p = 0;
    while (a != 0) {
        if ((a & 0x1) != 0) {
            p += b;
        }
        a >>= 1;
        b <<= 1;
    }
    return p;
}