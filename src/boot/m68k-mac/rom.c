#include <stdint.h>
#include <boot/m68k-mac/rom.h>

extern void _BiRomInitGraf(BiQdGrafPort **port);

void BiRomInitGraf(BiQdGrafPort *port) {
    _BiRomInitGraf(&port);
}
