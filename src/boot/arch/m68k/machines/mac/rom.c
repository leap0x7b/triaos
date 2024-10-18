#include <stdint.h>
#include <boot/arch/m68k/machines/mac/rom.h>

extern void _BiRomInitGraf(BiQdGrafPort **port);

void BiRomInitGraf(BiQdGrafPort *port) {
    _BiRomInitGraf(&port);
}
