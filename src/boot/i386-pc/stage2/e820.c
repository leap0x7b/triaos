#include <stdint.h>
#include <lib/e9.h>
#include <boot/i386-pc/e820.h>
#include <boot/i386-pc/real.h>

BeMemmapEntry *e820_map[E820_MAX_ENTRIES];
size_t e820_entries = 0;

void BiE820Init(void) {
    BiRealRegisters regs = {0};

    for (size_t i = 0; i < E820_MAX_ENTRIES; i++) {
        BeMemmapEntry entry;

        regs.eax = 0xE820;
        regs.ecx = 24;
        regs.edx = 0x534D4150;
        regs.edi = (uint32_t)&entry;
        BiRealInterrupt(0x15, &regs, &regs);

        if (regs.eflags & EFLAGS_CF) {
            e820_entries = i;
            return;
        }

        e820_map[i] = &entry;

        if (!regs.ebx) {
            e820_entries = ++i;
            return;
        }
    }
}
