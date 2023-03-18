#ifndef BOOT__BOOT_INFO_H
#define BOOT__BOOT_INFO_H

#include <stdint.h>
#include <stddef.h>

typedef enum memmap_entry_type {
    MEMMAP_USABLE = 1,
    MEMMAP_RESERVED = 2,
    MEMMAP_ACPI_RECLAIMABLE = 3,
    MEMMAP_ACPI_NVS = 4,
    MEMMAP_CORRUPTED = 5,
} memmap_entry_type_t;

typedef struct memmap_entry {
    uint64_t base;
    uint64_t length;
    memmap_entry_type_t type;
    uint32_t unused;
} memmap_entry_t;

typedef struct memmap {
    size_t entry_count;
    memmap_entry_t **entries;
} memmap_t;

typedef enum framebuffer_memory_model {
    MEMORY_MODEL_RGB = 1,
    MEMORY_MODEL_BGR = 2,
} framebuffer_memory_model_t;

typedef struct framebuffer {
    uint64_t address;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint16_t bpp;
    framebuffer_memory_model_t memory_model;
    uint8_t red_mask_size;
    uint8_t red_mask_shift;
    uint8_t green_mask_size;
    uint8_t green_mask_shift;
    uint8_t blue_mask_size;
    uint8_t blue_mask_shift;
} framebuffer_t;

typedef struct boot_info {
    uint32_t magic; // 0x761ab007 ("triaboot" spelled in hexadecimal)
    memmap_t memmap;
    framebuffer_t framebuffer;
} boot_info_t;

#endif
