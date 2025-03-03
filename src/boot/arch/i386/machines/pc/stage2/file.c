#include <lib/string.h>
#include <boot/fs/fat32.h>
#include <boot/file.h>
#include <boot/arch/i386/machines/pc/disk.h>
#include <boot/arch/i386/machines/pc/partition.h>

int fopen(FILE *ret, BiPartition *partition, const char *filename) {
    ret->is_memfile = false;

    if (BiFsFat32CheckSignature(partition)) {
        BiFsFat32FileHandle *fd = BiFsFat32Open(partition, filename);

        ret->fd = (void *)fd;
        ret->read = (BiFileReadFn)BiFsFat32Read;
        ret->size = fd->size;

        return 0;
    }

    return -1;
}

int fread(FILE *fd, void *buf, uint64_t loc, uint64_t count) {
    if (fd->is_memfile) {
        memcpy(buf, (void *)((uintptr_t)fd->fd + loc), count);
        return 0;
    } else {
        return fd->read(fd->fd, buf, loc, count);
    }
}
