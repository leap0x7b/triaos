#pragma once
#include <stdint.h>

// SysError
void rom_syserror(uint16_t code);

// QuickDraw
typedef struct {
    uint8_t patterns[8];
} pattern_t;

typedef struct {
    int16_t top;
    int16_t left;
    int16_t bottom;
    int16_t right;
} rect_t;

typedef struct {
    void *address;
    uint16_t row_bytes;
    rect_t bounds;
} bitmap_t;

typedef struct {
    uint16_t v;
    uint16_t h;
} point_t;

typedef struct Cursor {
    int16_t data[16];
    int16_t mask[16];
    point_t hot_spot;
} cursor_t;

// https://developer.apple.com/library/archive/documentation/mac/pdf/ImagingWithQuickDraw.pdf
typedef struct {
    uint8_t _internal[76];
    int32_t randdom_seed;
    bitmap_t bitmap;
    cursor_t arrow;
    pattern_t dark_gray;
    pattern_t light_gray;
    pattern_t gray;
    pattern_t black;
    pattern_t white;
    void *port;
} grafport_t;

// https://developer.apple.com/library/archive/documentation/mac/pdf/Imaging_With_QuickDraw/Color_QuickDraw.pdf
typedef struct {
    void *address;
    int16_t row_bytes;
    rect_t bounds;
    int16_t verion;
    int16_t packing_type;
    uint32_t packed_size;
    int32_t width;
    int32_t height;
    int16_t pixel_type;
    int16_t pixel_size;
    int16_t component_count;
    int16_t component_size;
    int32_t plane_bytes;
    void *table;
    int32_t _reserved;
} pixmap_t;

// https://developer.apple.com/library/archive/documentation/mac/pdf/Imaging_With_QuickDraw/Graphics_Devices.pdf
typedef struct {
    int16_t width;
    int16_t depth;
    void *handle;
    void *mask;
} cursor_data_t;

// https://developer.apple.com/library/archive/documentation/mac/pdf/Imaging_With_QuickDraw/Graphics_Devices.pdf
typedef struct {
    int16_t reference_number;
    int16_t id;
    int16_t type;
    void *inverse_table;
    int16_t preferred_resolution;
    void *search_proc;
    void *complement_proc;
    int16_t flags;
    pixmap_t **pixmap;
    int32_t reference_value;
    int32_t next_gdevice;
    rect_t rect;
    int32_t mode;
    cursor_data_t cursor_data;
    int32_t _reserved;
} gdevice_t;

void rom_initgraf(grafport_t *port);
