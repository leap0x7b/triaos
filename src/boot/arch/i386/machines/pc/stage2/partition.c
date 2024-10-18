#include <stddef.h>
#include <stdint.h>
#include <boot/alloc.h>
#include <boot/arch/i386/machines/pc/partition.h>
#include <boot/arch/i386/machines/pc/disk.h>

struct mbr_entry {
	uint8_t status;
	uint8_t chs_first_sector[3];
	uint8_t type;
	uint8_t chs_last_sector[3];
	uint32_t first_sector;
	uint32_t sector_count;
} __attribute__((packed));

static int get_logical_part(BiPartition *ret, BiPartition *extended_part,
                                int drive, int partition) {
    struct mbr_entry entry;

    size_t ebr_sector = 0;

    for (int i = 0; i < partition; i++) {
        size_t entry_offset = ebr_sector * extended_part->sector_size + 0x1ce;

        int r;
        r = BiReadPartition(extended_part, &entry, entry_offset, sizeof(struct mbr_entry));
        if (r)
            return r;

        if (entry.type != 0x0f && entry.type != 0x05)
            return END_OF_TABLE;

        ebr_sector = entry.first_sector;
    }

    size_t entry_offset = ebr_sector * extended_part->sector_size + 0x1be;

    int r;
    r = BiReadPartition(extended_part, &entry, entry_offset, sizeof(struct mbr_entry));
    if (r)
        return r;

    if (entry.type == 0)
        return NO_PARTITION;

    ret->drive = drive;
    ret->partition = partition + 4;
    ret->sector_size = BiDiskGetSectorSize(drive);
    ret->first_sector = extended_part->first_sector + ebr_sector + entry.first_sector;
    ret->sector_count = entry.sector_count;

    return 0;
}

int BiGetPartition(BiPartition *ret, int drive, int partition) {
    // Check if actually valid mbr
    uint16_t hint;
    BiDiskReadBytes(drive, &hint, 444, sizeof(uint16_t));
    if (hint && hint != 0x5A5A)
        return INVALID_TABLE;

    struct mbr_entry entry;

    if (partition > 3) {
        for (int i = 0; i < 4; i++) {
            size_t entry_offset = 0x1BE + sizeof(struct mbr_entry) * i;

            int r = BiDiskReadBytes(drive, &entry, entry_offset, sizeof(struct mbr_entry));
            if (r)
                return r;

            if (entry.type != 0x0f)
                continue;

            BiPartition extended_part;

            extended_part.drive = drive;
            extended_part.partition = i;
            extended_part.sector_size = BiDiskGetSectorSize(drive);
            extended_part.first_sector = entry.first_sector;
            extended_part.sector_count = entry.sector_count;

            return get_logical_part(ret, &extended_part, drive, partition - 4);
        }

        return END_OF_TABLE;
    }

    size_t entry_offset = 0x1be + sizeof(struct mbr_entry) * partition;

    int r = BiDiskReadBytes(drive, &entry, entry_offset, sizeof(struct mbr_entry));
    if (r)
        return r;

    if (entry.type == 0)
        return NO_PARTITION;

    ret->drive = drive;
    ret->partition = partition;
    ret->sector_size = BiDiskGetSectorSize(drive);
    ret->first_sector = entry.first_sector;
    ret->sector_count = entry.sector_count;

    return 0;
}

int BiReadPartition(BiPartition *part, void *buffer, uint64_t location, uint64_t count) {
    return BiDiskReadBytes(part->drive, buffer,
                     location + (part->first_sector * part->sector_size), count);
}
