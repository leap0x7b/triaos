#include <stdint.h>
#include <lib/string.h>
#include <boot/m68k-mac/rom.h>

void *bss_start;
void *bss_end;

void main(void) {
    memset((void *)&bss_start, 0, (uint32_t)&bss_end - (uint32_t)&bss_start);

    grafport_t grafport;
    rom_initgraf(&grafport);
    memset(grafport.bitmap.address, 0, (grafport.bitmap.bounds.bottom - grafport.bitmap.bounds.top) * (grafport.bitmap.row_bytes & 0x3fff));

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

unsigned long long __muldi3(unsigned long long a, unsigned long long b) {
    unsigned long long p = 0;
    while (a != 0) {
        if ((a & 0x1) != 0) {
            p += b;
        }
        a >>= 1;
        b <<= 1;
    }
    return p;
}
