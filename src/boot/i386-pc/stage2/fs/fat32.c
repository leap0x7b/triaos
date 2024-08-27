#include <lib/ctype.h>
#include <lib/misc.h>
#include <lib/string.h>
#include <lib/fs/fat32.h>
#include <boot/alloc.h>
#include <boot/fs/fat32.h>
#include <boot/i386-pc/disk.h>
#include <boot/i386-pc/partition.h>

static int open_in(BiFsFat32Context *context, TiFsFat32DirectoryEntry *directory, TiFsFat32DirectoryEntry *file, const char *name);

static int init_context(BiFsFat32Context *context, BiPartition *partition) {
    context->partition = partition;

    TiFsFat32Bpb bpb;
    BiReadPartition(context->partition, &bpb, 0, sizeof(TiFsFat32Bpb));

    // Checks for FAT12/16
    if (strncmp((void *)((uintptr_t)&bpb + 0x36), "FAT", 3) != 0) {
        return 1;
    }

    // Checks for FAT32
    if (strncmp((void *)((uintptr_t)&bpb + 0x52), "FAT", 3) != 0) {
        return 1;
    }

    // Checks for FAT32 (with 64-bit sector count)
    if (strncmp((void *)((uintptr_t)&bpb + 0x03), "FAT32", 5) != 0) {
        return 1;
    }

    size_t root_dir_sects = ((bpb.root_entries_count * 32) + (bpb.bytes_per_sector - 1)) / bpb.bytes_per_sector;
    size_t data_sects = (bpb.sectors_count ? bpb.sectors_count : bpb.large_sectors_count) - (bpb.reserved_sectors + (bpb.fats_count * (bpb.sectors_per_fat ? bpb.sectors_per_fat: bpb.sectors_per_fat)) + root_dir_sects);
    size_t clusters_count = data_sects / bpb.sectors_per_cluster;

    if (clusters_count < 4085) {
        context->type = 12;
    } else if (clusters_count < 65525) {
        context->type = 16;
    } else {
        context->type = 32;
    }

    context->bytes_per_sector = bpb.bytes_per_sector;
    context->sectors_per_cluster = bpb.sectors_per_cluster;
    context->reserved_sectors = bpb.reserved_sectors;
    context->number_of_fats = bpb.fats_count;
    context->hidden_sectors = bpb.hidden_sectors_count;
    context->sectors_per_fat = context->type == 32 ? bpb.sectors_per_fat32 : bpb.sectors_per_fat;
    context->root_directory_cluster = bpb.root_directory_cluster;
    context->fat_start_lba = bpb.reserved_sectors;
    context->root_entries = bpb.root_entries_count;
    context->root_start = context->reserved_sectors + context->number_of_fats * context->sectors_per_fat;
    context->root_size = DIV_ROUNDUP(context->root_entries * sizeof(TiFsFat32DirectoryEntry), context->bytes_per_sector);
    switch (context->type) {
        case 12:
        case 16:
            context->data_start_lba = context->root_start + context->root_size;
            break;
        case 32:
            context->data_start_lba = context->root_start;
            break;
        default:
            __builtin_unreachable();
    }

    // get the volume label
    TiFsFat32DirectoryEntry _current_directory;
    TiFsFat32DirectoryEntry *current_directory;

    switch (context->type) {
        case 12:
        case 16:
            current_directory = NULL;
            break;
        case 32:
            _current_directory.cluster_num_low = context->root_directory_cluster & 0xFFFF;
            _current_directory.cluster_num_high = context->root_directory_cluster >> 16;
            current_directory = &_current_directory;
            break;
        default:
            __builtin_unreachable();
    }

    char *vol_label;
    if (open_in(context, current_directory, (TiFsFat32DirectoryEntry *)&vol_label, NULL) == 0) {
        context->label = vol_label;
    } else {
        context->label = NULL;
    }

    return 0;
}

static int read_cluster_from_map(BiFsFat32Context *context, uint32_t cluster, uint32_t *out) {
    switch (context->type) {
        case 12: {
            *out = 0;
            uint16_t tmp = 0;
            BiReadPartition(context->partition, &tmp, context->fat_start_lba * context->bytes_per_sector + (cluster + cluster / 2), sizeof(uint16_t));
            if (cluster % 2 == 0) {
                *out = tmp & 0xfff;
            } else {
                *out = tmp >> 4;
            }
            break;
        }
        case 16:
            *out = 0;
            BiReadPartition(context->partition, out, context->fat_start_lba * context->bytes_per_sector + cluster * sizeof(uint16_t), sizeof(uint16_t));
            break;
        case 32:
            BiReadPartition(context->partition, out, context->fat_start_lba * context->bytes_per_sector + cluster * sizeof(uint32_t), sizeof(uint32_t));
            *out &= 0x0fffffff;
            break;
        default:
            __builtin_unreachable();
    }

    return 0;
}

static uint32_t *cache_cluster_chain(BiFsFat32Context *context,
                                     uint32_t initial_cluster,
                                     size_t *_chain_length) {
    uint32_t cluster_limit = (context->type == 12 ? 0xfef     : 0)
                           | (context->type == 16 ? 0xffef    : 0)
                           | (context->type == 32 ? 0xfffffef : 0);
    if (initial_cluster < 0x2 || initial_cluster > cluster_limit)
        return NULL;
    uint32_t cluster = initial_cluster;
    size_t chain_length;
    for (chain_length = 1; ; chain_length++) {
        read_cluster_from_map(context, cluster, &cluster);
        if (cluster < 0x2 || cluster > cluster_limit)
            break;
    }
    uint32_t *cluster_chain = BiAllocate(chain_length * sizeof(uint32_t));
    cluster = initial_cluster;
    for (size_t i = 0; i < chain_length; i++) {
        cluster_chain[i] = cluster;
        read_cluster_from_map(context, cluster, &cluster);
    }
    *_chain_length = chain_length;
    return cluster_chain;
}

static bool read_cluster_chain(BiFsFat32Context *context,
                               uint32_t *cluster_chain,
                               void *buf, uint64_t location, uint64_t count) {
    size_t block_size = context->sectors_per_cluster * context->bytes_per_sector;
    for (uint64_t progress = 0; progress < count;) {
        uint64_t block = (location + progress) / block_size;

        uint64_t chunk = count - progress;
        uint64_t offset = (location + progress) % block_size;
        if (chunk > block_size - offset)
            chunk = block_size - offset;

        uint64_t base = ((uint64_t)context->data_start_lba + (cluster_chain[block] - 2) * context->sectors_per_cluster) * context->bytes_per_sector;
        BiReadPartition(context->partition, (void *)((uintptr_t)buf + progress), base + offset, chunk);

        progress += chunk;
    }

    return true;
}

// Copy ucs-2 characters to char *
static void fat32_lfncpy(char *destination, const void *source, int size) {
    for (int i = 0; i < size; i++) {
        // ignore high bytes
        *(((uint8_t *)destination) + i) = *(((uint8_t *)source) + (i * 2));
    }
}

static bool fat32_filename_to_8_3(char *dest, const char *src) {
    int i = 0, j = 0;
    bool ext = false;

    for (size_t k = 0; k < 8+3; k++)
        dest[k] = ' ';

    while (src[i]) {
        if (src[i] == '.') {
            if (ext) {
                // This is a double extension here, just give up.
                return false;
            }
            ext = true;
            j = 8;
            i++;
            continue;
        }
        if (j >= 8+3 || (j >= 8 && !ext)) {
            // Filename too long, give up.
            return false;
        }
        dest[j++] = toupper(src[i++]);
    }

    return true;
}

static int open_in(BiFsFat32Context *context, TiFsFat32DirectoryEntry *directory, TiFsFat32DirectoryEntry *file, const char *name) {
    size_t block_size = context->sectors_per_cluster * context->bytes_per_sector;
    char current_lfn[FAT32_LFN_MAX_FILENAME_LENGTH] = {0};

    size_t dir_chain_len;
    TiFsFat32DirectoryEntry *directory_entries;

    if (directory != NULL) {
        uint32_t current_cluster_number = directory->cluster_num_low;
        if (context->type == 32)
            current_cluster_number |= (uint32_t)directory->cluster_num_high << 16;

        uint32_t *directory_cluster_chain = cache_cluster_chain(context, current_cluster_number, &dir_chain_len);

        if (directory_cluster_chain == NULL)
            return -1;

        directory_entries = BiAllocate(dir_chain_len * block_size);

        read_cluster_chain(context, directory_cluster_chain, directory_entries, 0, dir_chain_len * block_size);

        //BiFree(directory_cluster_chain, dir_chain_len * sizeof(uint32_t));
    } else {
        dir_chain_len = DIV_ROUNDUP(context->root_entries * sizeof(TiFsFat32DirectoryEntry), block_size);

        directory_entries = BiAllocate(dir_chain_len * block_size);

        BiReadPartition(context->partition, directory_entries, context->root_start * context->bytes_per_sector, context->root_entries * sizeof(TiFsFat32DirectoryEntry));
    }

    int ret;

    for (size_t i = 0; i < (dir_chain_len * block_size) / sizeof(TiFsFat32DirectoryEntry); i++) {
        if (directory_entries[i].filename[0] == 0x00) {
            // no more entries here
            break;
        }

        if (name == NULL) {
            if (directory_entries[i].attribute != FAT32_ATTRIBUTE_VOLLABEL) {
                continue;
            }
            char *r = BiAllocate(12);
            memcpy(r, directory_entries[i].filename, 11);
            // remove trailing spaces
            for (int j = 10; j >= 0; j--) {
                if (r[j] == ' ') {
                    r[j] = 0;
                    continue;
                }
                break;
            }
            *((char **)file) = r;
            ret = 0;
            goto out;
        }

        if (directory_entries[i].attribute == FAT32_LFN_ATTRIBUTE) {
            TiFsFat32LfnEntry* lfn = (TiFsFat32LfnEntry*) &directory_entries[i];

            if (lfn->sequence_number & 0x40) {
                // this lfn is the first entry in the table, clear the lfn buffer
                memset(current_lfn, ' ', sizeof(current_lfn));
            }

            const uint32_t lfn_index = ((lfn->sequence_number & 0x1f) - 1U) * 13U;
            if (lfn_index >= FAT32_LFN_MAX_ENTRIES * 13) {
                continue;
            }

            fat32_lfncpy(current_lfn + lfn_index + 00, lfn->name1, 5);
            fat32_lfncpy(current_lfn + lfn_index + 05, lfn->name2, 6);
            fat32_lfncpy(current_lfn + lfn_index + 11, lfn->name3, 2);

            if (lfn_index != 0)
                continue;

            // remove trailing spaces
            for (int j = SIZEOF_ARRAY(current_lfn) - 2; j >= -1; j--) {
                if (j == -1 || current_lfn[j] != ' ') {
                    current_lfn[j + 1] = 0;
                    break;
                }
            }

            if (strcmp(current_lfn, name) == 0) {
                *file = directory_entries[i+1];
                ret = 0;
                goto out;
            }
        }

        if (directory_entries[i].attribute & (1 << 3)) {
            // It is a volume label, skip
            continue;
        }
        // SFN
        char fn[8+3];
        if (!fat32_filename_to_8_3(fn, name)) {
            continue;
        }
        if (!strncmp(directory_entries[i].filename, fn, 8+3)) {
            *file = directory_entries[i];
            ret = 0;
            goto out;
        }
    }

    // file not found
    ret = -1;

out:
    //BiFree(directory_entries, dir_chain_len * block_size);
    return ret;
}

bool BiFsFat32CheckSignature(BiPartition *partition) {
    BiFsFat32Context context;
    return init_context(&context, partition) == 0;
}

char *BiFsFat32GetLabel(BiPartition *partition) {
    BiFsFat32Context context;
    if (init_context(&context, partition) != 0) {
        return NULL;
    }

    return context.label;
}

BiFsFat32FileHandle *BiFsFat32Open(BiPartition *part, const char *path) {
    BiFsFat32Context context;
    int r = init_context(&context, part);

    if (r) {
        return NULL;
    }

    TiFsFat32DirectoryEntry _current_directory;
    TiFsFat32DirectoryEntry *current_directory;
    TiFsFat32DirectoryEntry current_file;
    unsigned int current_index = 0;
    char current_part[FAT32_LFN_MAX_FILENAME_LENGTH];

    // skip trailing slashes
    while (path[current_index] == '/') {
        current_index++;
    }

    // walk down the directory tree
    switch (context.type) {
        case 12:
        case 16:
            current_directory = NULL;
            break;
        case 32:
            _current_directory.cluster_num_low = context.root_directory_cluster & 0xFFFF;
            _current_directory.cluster_num_high = context.root_directory_cluster >> 16;
            current_directory = &_current_directory;
            break;
        default:
            __builtin_unreachable();
    }

    for (;;) {
        bool expect_directory = false;

        for (unsigned int i = 0; i < SIZEOF_ARRAY(current_part); i++) {
            if (path[i + current_index] == 0) {
                memcpy(current_part, path + current_index, i);
                current_part[i] = 0;
                expect_directory = false;
                break;
            }

            if (path[i + current_index] == '/') {
                memcpy(current_part, path + current_index, i);
                current_part[i] = 0;
                current_index += i + 1;
                expect_directory = true;
                break;
            }
        }

        if ((r = open_in(&context, current_directory, &current_file, current_part)) != 0) {
            return NULL;
        }

        if (expect_directory) {
            _current_directory = current_file;
            current_directory = &_current_directory;
        } else {
            BiFsFat32FileHandle *ret = BiAllocate(sizeof(BiFsFat32FileHandle));

            ret->context = context;
            ret->first_cluster = current_file.cluster_num_low;
            if (context.type == 32)
                ret->first_cluster |= (uint64_t)current_file.cluster_num_high << 16;
            ret->size = current_file.size;
            ret->size_in_clusters = DIV_ROUNDUP(current_file.size, context.bytes_per_sector);
            ret->cluster_chain = cache_cluster_chain(&context, ret->first_cluster, &ret->chain_len);

            return ret;
        }
    }
}

int BiFsFat32Read(BiFsFat32FileHandle *file, void *buf, uint64_t location, uint64_t count) {
    return !read_cluster_chain(&file->context, file->cluster_chain, buf, location, count);
}
