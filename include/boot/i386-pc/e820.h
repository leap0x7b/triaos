#ifndef BOOT__I386_PC__E820_H
#define BOOT__I386_PC__E820_H

#include <boot/boot_info.h>

#define E820_MAX_ENTRIES 256

extern BeMemmapEntry *e820_map[E820_MAX_ENTRIES];
extern size_t e820_entries;

void BiE820Init(void);

#endif
