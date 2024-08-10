#ifndef BOOT__M68K_MAC__ROM_H
#define BOOT__M68K_MAC__ROM_H

#include <stdint.h>

// SysError
void BiRomSysError(uint16_t code);

// QuickDraw
typedef struct {
    uint8_t patterns[8];
} BiQdPattern;

typedef struct {
    int16_t top;
    int16_t left;
    int16_t bottom;
    int16_t right;
} BiQdRect;

typedef struct {
    void *address;
    uint16_t row_bytes;
    BiQdRect bounds;
} BiQdBitmap;

typedef struct {
    uint16_t v;
    uint16_t h;
} BiQdPoint;

typedef struct {
    int16_t data[16];
    int16_t mask[16];
    BiQdPoint hot_spot;
} BiQdCursor;

// https://developer.apple.com/library/archive/documentation/mac/pdf/ImagingWithQuickDraw.pdf
typedef struct {
    uint8_t _internal[76];
    int32_t random_seed;
    BiQdBitmap bitmap;
    BiQdCursor arrow;
    BiQdPattern dark_gray;
    BiQdPattern light_gray;
    BiQdPattern gray;
    BiQdPattern black;
    BiQdPattern white;
    void *port;
} BiQdGrafPort;

// https://developer.apple.com/library/archive/documentation/mac/pdf/Imaging_With_QuickDraw/Color_QuickDraw.pdf
typedef struct {
    void *address;
    int16_t row_bytes;
    BiQdRect bounds;
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
} BiQdPixmap;

// https://developer.apple.com/library/archive/documentation/mac/pdf/Imaging_With_QuickDraw/Graphics_Devices.pdf
typedef struct {
    int16_t width;
    int16_t depth;
    void *handle;
    void *mask;
} BiQdCursorData;

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
    BiQdPixmap **pixmap;
    int32_t reference_value;
    int32_t next_gdevice;
    BiQdRect rect;
    int32_t mode;
    BiQdCursorData cursor_data;
    int32_t _reserved;
} BiQdGDevice;

void BiRomInitGraf(BiQdGrafPort *port);

#endif
