#ifndef BOOT__ALLOC_H
#define BOOT__ALLOC_H

#include <stdint.h>
#include <stddef.h>

void *mem_alloc(size_t count);
void *mem_alloc_aligned(size_t count, size_t alignment);

#endif
