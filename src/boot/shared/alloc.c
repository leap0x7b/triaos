#include <stddef.h>
#include <boot/alloc.h>
#include <lib/string.h>
#include <lib/misc.h>

extern void *bss_end[];
static size_t heap_allocator_base = (size_t)bss_end;
static size_t heap_allocator_limit = 0x80000;

void *BiAllocate(size_t count) {
    return BiAllocateAligned(count, 4);
}

void *BiAllocateAligned(size_t count, size_t alignment) {
    size_t new_base = ALIGN_UP(heap_allocator_base, alignment);
    void *ret = (void *)new_base;
    new_base += count;

    if (new_base >= heap_allocator_limit)
        return NULL;

    heap_allocator_base = new_base;

    memset(ret, 0, count);
    return ret;
}
