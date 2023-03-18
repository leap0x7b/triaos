#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <boot/a20.h>
#include <lib/io.h>
#include <boot/real.h>

static inline uint16_t mminw(uintptr_t addr) {
    uint16_t ret;
    __asm__ volatile (
        "movw (%1), %0"
        : "=r" (ret)
        : "r"  (addr)
        : "memory"
    );
    return ret;
}

static inline void mmoutw(uintptr_t addr, uint16_t value) {
    __asm__ volatile (
        "movw %1, (%0)"
        :
        : "r" (addr), "ir" (value)
        : "memory"
    );
}

bool a20_check(void) {
    if (mminw(0x7dfe) != mminw(0x7dfe + 0x100000))
        return true;

    mmoutw(0x7dfe, ~mminw(0x7dfe));

    if (mminw(0x7dfe) != mminw(0x7dfe + 0x100000))
        return true;

    return false;
}

// Keyboard controller method code below taken from:
// https://wiki.osdev.org/A20_Line

bool a20_init(void) {
    if (a20_check())
        return true;

    // BIOS method
    real_regs_t regs = {0};
    regs.eax = 0x2401;
    real_int(0x15, &regs, &regs);

    if (a20_check())
        return true;

    // Keyboard controller method
    while (inb(0x64) & 2);
    outb(0x64, 0xad);
    while (inb(0x64) & 2);
    outb(0x64, 0xd0);
    while (!(inb(0x64) & 1));
    uint8_t b = inb(0x60);
    while (inb(0x64) & 2);
    outb(0x64, 0xd1);
    while (inb(0x64) & 2);
    outb(0x60, b | 2);
    while (inb(0x64) & 2);
    outb(0x64, 0xae);

    while (inb(0x64) & 2);

    if (a20_check())
        return true;

    // Fast A20 gate (very dangerous, so it's best to put it last)
    outb(0x92, 0x2);
    if (a20_check())
        return true;

    return false;
}
