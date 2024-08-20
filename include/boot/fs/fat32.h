#ifndef BOOT__FS__FAT32_H
#define BOOT__FS__FAT32_H

#include <stdint.h>
#include <stddef.h>
#include <boot/i386-pc/partition.h>

typedef struct {
    BiPartition *partition;
    int type;
    char *label;
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t number_of_fats;
    uint32_t hidden_sectors;
    uint32_t sectors_per_fat;
    uint32_t fat_start_lba;
    uint32_t data_start_lba;
    uint32_t root_directory_cluster;
    uint16_t root_entries;
    uint32_t root_start;
    uint32_t root_size;
} BiFsFat32Context;

typedef struct {
    BiFsFat32Context context;
    uint32_t first_cluster;
    uint32_t size;
    uint32_t size_in_clusters;
    uint32_t *cluster_chain;
    size_t chain_len;
} BiFsFat32FileHandle;

bool BiFsFat32CheckSignature(BiPartition *part);
char *BiFsFat32GetLabel(BiPartition *partition);
BiFsFat32FileHandle *BiFsFat32Open(BiPartition *partition, const char *path);
int BiFsFat32Read(BiFsFat32FileHandle *file, void *buf, uint64_t location, uint64_t count);

#endif
