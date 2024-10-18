#ifndef BOOT__FILE_H
#define BOOT__FILE_H

#include <stdint.h>
#include <stdbool.h>
#include <boot/arch/i386/machines/pc/partition.h>

typedef int (*BiFileReadFn)(void *, void *, uint64_t, uint64_t);

typedef struct _FILE {
    bool is_memfile;
    void *fd;
    BiFileReadFn read;
    uint64_t size;
} FILE;

int fopen(FILE *ret, BiPartition *partition, const char *filename);
int fread(FILE *fd, void *buf, uint64_t location, uint64_t count);
void *freadall(FILE *fd, uint32_t type);

#endif
