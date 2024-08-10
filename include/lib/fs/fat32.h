#ifndef LIB__FS__FAT32_H
#define LIB__FS__FAT32_H

#include <stdint.h>

#define FAT32_LFN_MAX_ENTRIES 20
#define FAT32_LFN_MAX_FILENAME_LENGTH (FAT32_LFN_MAX_ENTRIES * 13 + 1)

#define FAT32_ATTRIBUTE_SUBDIRECTORY 0x10
#define FAT32_LFN_ATTRIBUTE 0x0F
#define FAT32_ATTRIBUTE_VOLLABEL 0x08

typedef struct {
    uint8_t jmp[3];
    char oem_name[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t fats_count;
    uint16_t root_entries_count;
    uint16_t sectors_count;
    uint8_t media_descriptor_type;
    uint16_t sectors_per_fat;
    uint16_t sectors_per_track;
    uint16_t heads_count;
    uint32_t hidden_sectors;
    uint32_t large_sector_count;
    uint32_t sectors_per_fat32;
    uint16_t flags;
    uint16_t fat_version_number;
    uint32_t root_directory_cluster;
    uint16_t fsinfo_sector;
    uint16_t backup_boot_sector;
    uint8_t reserved1[12];
    uint8_t drive_number;
    uint8_t reserved2;
    uint8_t boot_signature;
    uint32_t serial_number;
    char volume_label[11];
    char file_system[8];
} __attribute__((packed)) TiFsFat32Bpb;

typedef struct {
    char filename[8];
    char extension[3];
    uint8_t attribute;
    uint8_t data1[8];
    uint16_t cluster_num_high;
    uint8_t data2[4];
    uint16_t cluster_num_low;
    uint32_t size;
} __attribute__((packed)) TiFsFat32DirectoryEntry;

typedef struct {
    uint8_t sequence_number;
    char name1[10];
    uint8_t attribute;
    uint8_t type;
    uint8_t dos_checksum;
    char name2[12];
    uint16_t first_cluster;
    char name3[4];
} __attribute__((packed)) TiFsFat32LfnEntry;

#endif
