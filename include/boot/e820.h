#pragma once
#include <stdint.h>
#include <boot/boot_info.h>

extern memmap_entry_t e820_map[];
extern size_t e820_entries;

void e820_init(void);
