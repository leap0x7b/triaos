#ifndef E820_H
#define E820_H

#include <stdint.h>
#include <boot/boot_info.h>

#define E820_MAX_ENTRIES 256

extern memmap_entry_t *e820_map[E820_MAX_ENTRIES];
extern size_t e820_entries;

void e820_init(void);

#endif
