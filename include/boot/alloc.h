#ifndef BOOT__ALLOC_H
#define BOOT__ALLOC_H

#include <stdint.h>
#include <stddef.h>

void *BiAllocate(size_t count);
void *BiAllocateAligned(size_t count, size_t alignment);

#endif
