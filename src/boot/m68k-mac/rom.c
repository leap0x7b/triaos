#include <stdint.h>
#include <boot/m68k-mac/rom.h>

extern void _rom_initgraf(grafport_t **port);

void rom_initgraf(grafport_t *port) {
    _rom_initgraf(&port);
}
