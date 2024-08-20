#ifndef BOOT__I386_PC__PART_H
#define BOOT__I386_PC__PART_H

#include <stdint.h>
#include <stdbool.h>

#define NO_PARTITION (-1)
#define INVALID_TABLE (-2)
#define END_OF_TABLE (-3)

typedef struct {
    int drive;
    int partition;
    int sector_size;
    uint64_t first_sector;
    uint64_t sector_count;
} BiPartition;

void BiPartitionCreateIndex(void);
int BiGetPartition(BiPartition *part, int drive, int partition);
int BiReadPartition(BiPartition *part, void *buffer, uint64_t location, uint64_t count);

#endif
