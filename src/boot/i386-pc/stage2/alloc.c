#include <stdint.h>
#include <stddef.h>
#include <boot/alloc.h>
#include <lib/string.h>
#include <lib/misc.h>

extern void *stack_end[];
static size_t heap_allocator_base = (size_t)stack_end;
static size_t heap_allocator_limit = 0x80000;

void *mem_alloc(size_t count) {
    return mem_alloc_aligned(count, 4);
}

void *mem_alloc_aligned(size_t count, size_t alignment) {
    size_t new_base = ALIGN_UP(heap_allocator_base, alignment);
    void *ret = (void *)new_base;
    new_base += count;
    /*if (new_base >= heap_allocator_limit)
        panic("Memory allocation failed");*/
    heap_allocator_base = new_base;

    // Zero out allocated space
    memset(ret, 0, count);

    return ret;
}
