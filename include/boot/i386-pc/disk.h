#ifndef BOOT__DISK_H
#define BOOT__DISK_H

#include <stdint.h>

typedef struct {
    uint16_t buffer_size;
    uint16_t info_flags;
    uint32_t cylinders;
    uint32_t heads;
    uint32_t sectors;
    uint64_t lba_count;
    uint16_t bytes_per_sector;
    uint32_t edd;
} __attribute__((packed)) BiDriveParams;

int BiDiskGetSectorSize(int drive);
int BiDiskReadBytes(int drive, void *buffer, uint64_t loc, uint64_t count);

#endif
