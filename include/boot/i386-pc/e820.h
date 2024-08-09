#ifndef BOOT__E820_H
#define BOOT__E820_H

#include <stdint.h>
#include <boot/boot_info.h>

#define E820_MAX_ENTRIES 256

extern BeMemmapEntry *e820_map[E820_MAX_ENTRIES];
extern size_t e820_entries;

void BiE820Init(void);

#endif
