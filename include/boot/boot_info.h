#ifndef BOOT__BOOT_INFO_H
#define BOOT__BOOT_INFO_H

#include <stdint.h>
#include <stddef.h>

#define TRIABOOT_MAGIC 0x761AB007

typedef enum {
    MEMMAP_USABLE = 1,
    MEMMAP_RESERVED = 2,
    MEMMAP_ACPI_RECLAIMABLE = 3,
    MEMMAP_ACPI_NVS = 4,
    MEMMAP_CORRUPTED = 5,
} BeMemmapEntryType;

typedef struct {
    uint64_t address;
    uint64_t size;
    BeMemmapEntryType type;
    uint32_t unused;
} BeMemmapEntry;

typedef struct {
    size_t entry_count;
    BeMemmapEntry **entries;
} BeMemmap;

typedef enum {
    FRAMEBUFFER_MEMORY_MODEL_RGB = 1,
    FRAMEBUFFER_MEMORY_MODEL_BGR = 2,
    FRAMEBUFFER_MEMORY_MODEL_RGBA = 3,
    FRAMEBUFFER_MEMORY_MODEL_BGRA = 4,
} BeFramebufferMemoryModel;

typedef struct {
    uint64_t address;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint16_t bpp;
    BeFramebufferMemoryModel memory_model;
    uint8_t red_mask_size;
    uint8_t red_mask_shift;
    uint8_t green_mask_size;
    uint8_t green_mask_shift;
    uint8_t blue_mask_size;
    uint8_t blue_mask_shift;
} BeFramebuffer;

typedef struct {
    uint32_t magic; // 0x761AB007 ("triaboot" spelled in hexadecimal)
    char *command_line;
    char *bootloader_name;
    BeMemmap memmap;
} BeBootInfo;

#endif
