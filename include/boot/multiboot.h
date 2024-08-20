#ifndef BOOT__MULTIBOOT_H
#define BOOT__MULTIBOOT_H

#include <stdint.h>
#include <boot/boot_info.h>

#define MULTIBOOT_HEADER_MAGIC 0x1BADB002
#define MULTIBOOT_BOOTLOADER_MAGIC 0x2BADB002

typedef struct {
    uint32_t size;
    uint64_t address;
    uint64_t entry_size;
    BeMemmapEntryType type;
} BeMultibootMemmapEntry;

typedef struct {
    uint32_t control_info;
    uint32_t mode_info;
    uint16_t mode;
    uint16_t interface_segment;
    uint16_t interface_offset;
    uint16_t interface_size;
} BeMultibootVbe;

typedef enum {
    MULTIBOOT_FRAMEBUFFER_TYPE_INDEXED = 0,
    MULTIBOOT_FRAMEBUFFER_TYPE_RGB = 1,
    MULTIBOOT_FRAMEBUFFER_TYPE_TEXT_MODE = 2,
} BeMultibootFramebufferType;

typedef struct {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} BeMultibootColor;

typedef struct {
    uint64_t address;
    uint32_t pitch;
    uint32_t width;
    uint32_t height;
    uint8_t bpp;
    BeMultibootFramebufferType type;
    uint8_t red_mask_size;
    uint8_t red_mask_shift;
    uint8_t green_mask_size;
    uint8_t green_mask_shift;
    uint8_t blue_mask_size;
    uint8_t blue_mask_shift;

    union {
        struct {
            uint32_t framebuffer_palette_address;
            uint16_t framebuffer_palette_color_count;
        } _indexed;

        struct {
            uint8_t framebuffer_red_field_position;
            uint8_t framebuffer_red_mask_size;
            uint8_t framebuffer_green_field_position;
            uint8_t framebuffer_green_mask_size;
            uint8_t framebuffer_blue_field_position;
            uint8_t framebuffer_blue_mask_size;
        } _rgb;
    } _u;
} BeMultibootFramebuffer;

typedef struct {
    uint32_t magic; // 0x1BADB002
    uint32_t flags;
    uint32_t checksum;

    uint32_t header_addr;
    uint32_t load_addr;
    uint32_t load_end_addr;
    uint32_t bss_end_addr;
    uint32_t entry_addr;

    uint32_t mode_type;
    uint32_t width;
    uint32_t height;
    uint32_t depth;
} BeMultibootHeader;

typedef struct {
    uint32_t symbol_table_size;
    uint32_t string_table_size;
    uint32_t address;
    uint32_t _reserved;
} BeMultibootAOutSymbolTable;

typedef struct {
    uint32_t entry_count;
    uint32_t entry_size;
    uint32_t address;
    uint32_t section_index;
} BeMultibootElfSectionHeaderTable;

typedef struct {
    uint32_t flags;

    uint32_t memory_size_lower;
    uint32_t memory_size_upper;

    char *boot_device;
    char *command_line;

    uint32_t module_count;
    uint32_t module_address;

    union {
        BeMultibootAOutSymbolTable aout_symbol_table;
        BeMultibootElfSectionHeaderTable elf_section_header_table;
    } _u;

    uint32_t memmap_size;
    uint32_t memmap_address;

    uint32_t drive_info_size;
    uint32_t drive_info_address;

    uint32_t config_table;

    char *bootloader_name;

    uint32_t apm_table;

    BeMultibootVbe vbe;
    BeMultibootFramebuffer framebuffer;
} BeMultibootInfo;

#endif
