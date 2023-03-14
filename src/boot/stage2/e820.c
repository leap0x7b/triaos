#include <stdint.h>
#include <lib/e9.h>
#include <boot/e820.h>
#include <boot/real.h>

memmap_entry_t *e820_map[E820_MAX_ENTRIES];
size_t e820_entries = 0;

void e820_init(void) {
    real_regs_t regs = {0};

    for (size_t i = 0; i < E820_MAX_ENTRIES; i++) {
        memmap_entry_t entry;

        regs.eax = 0xe820;
        regs.ecx = 24;
        regs.edx = 0x534d4150;
        regs.edi = (uint32_t)&entry;
        real_int(0x15, &regs, &regs);

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
