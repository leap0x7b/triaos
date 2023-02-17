#include <stdint.h>
#include <lib/e9.h>
#include <boot/e820.h>
#include <boot/real.h>

#define MAX_ENTRIES 256

memmap_entry_t e820_map[MAX_ENTRIES];
size_t e820_entries = 0;

void e820_init(void) {
    rm_regs_t r = {0};

    for (size_t i = 0; i < MAX_ENTRIES; i++) {
        memmap_entry_t entry;

        r.eax = 0xe820;
        r.ecx = 24;
        r.edx = 0x534d4150;
        r.edi = (uint32_t)&entry;
        rm_int(0x15, &r, &r);

        if (r.eflags & EFLAGS_CF) {
            e820_entries = i;
            return;
        }

        e820_map[i] = entry;

        if (!r.ebx) {
            e820_entries = ++i;
            return;
        }
    }
}
