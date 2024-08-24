#include <stdint.h>
#include <stddef.h>
#include <boot/i386-pc/disk.h>
#include <boot/i386-pc/real.h>
#include <boot/alloc.h>
#include <lib/string.h>
#include <lib/misc.h>
#include <lib/i386-pc/e9.h>
#include <lib/i386-pc/console.h>

#define BLOCK_SIZE_IN_SECTORS 16
#define BLOCK_SIZE (sector_size * BLOCK_SIZE_IN_SECTORS)

#define CACHE_INVALID (~((uint64_t)0))

#define MAX_CACHE 16384

static int cached_drive = -1;
static uint8_t *cache = NULL;
static uint64_t cached_block = CACHE_INVALID;

struct dap {
    uint16_t size;
    uint16_t count;
    uint16_t offset;
    uint16_t segment;
    uint64_t lba;
};

static struct dap *dap = NULL;

static int cache_block(int drive, uint64_t block, int sector_size) {
    if (drive == cached_drive && block == cached_block)
        return 0;

    if (!dap) {
        dap = BiAllocate(sizeof(struct dap));
        dap->size = 16;
        dap->count = BLOCK_SIZE_IN_SECTORS;
    }

    if (!cache)
        cache = BiAllocateAligned(MAX_CACHE, 16);

    dap->segment = BiRealSegment(cache);
    dap->offset = BiRealOffset(cache);
    dap->lba = block * BLOCK_SIZE_IN_SECTORS;

    BiRealRegisters regs = {0};
    regs.eax = 0x4200;
    regs.edx = drive;
    regs.esi = (uint32_t)BiRealOffset(dap);
    regs.ds = BiRealSegment(dap);

    BiRealInterrupt(0x13, &regs, &regs);

    if (regs.eflags & EFLAGS_CF) {
        int ah = (regs.eax >> 8) & 0xFF;
        TiE9Printf("Disk read error: 0x%.4x (drive number %d)\n", ah, drive);
        TiConsolePrintf("!%.4X", ah);
        while (1)
            __asm__ volatile ("hlt");
        __builtin_unreachable();
    }

    cached_block = block;
    cached_drive = drive;

    return 0;
}

int BiDiskGetSectorSize(int drive) {
    BiRealRegisters regs = {0};
    BiDriveParams drive_params;

    regs.eax = 0x4800;
    regs.edx = drive;
    regs.ds = BiRealSegment(&drive_params);
    regs.esi = BiRealOffset(&drive_params);

    drive_params.buffer_size = sizeof(BiDriveParams);

    BiRealInterrupt(0x13, &regs, &regs);

    if (regs.eflags & EFLAGS_CF) {
        int ah = (regs.eax >> 8) & 0xFF;
        TiE9Printf("Disk read error: 0x%.4x (drive number %d)\n", ah, drive);
        TiConsolePrintf("!%.4X", ah);
        while (1)
            __asm__ volatile ("hlt");
        __builtin_unreachable();
    }

    return drive_params.bytes_per_sector;
}

int BiDiskReadBytes(int drive, void *buffer, uint64_t loc, uint64_t count) {
    int sector_size = BiDiskGetSectorSize(drive);

    uint64_t progress = 0;
    while (progress < count) {
        uint64_t block = (loc + progress) / BLOCK_SIZE;

        int ret;
        if ((ret = cache_block(drive, block, sector_size)))
            return ret;

        uint64_t chunk = count - progress;
        uint64_t offset = (loc + progress) % BLOCK_SIZE;
        if (chunk > BLOCK_SIZE - offset)
            chunk = BLOCK_SIZE - offset;

        memcpy((void *)((uint64_t)buffer + progress), &cache[offset], chunk);
        progress += chunk;
    }

    return 0;
}
