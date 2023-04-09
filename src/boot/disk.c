#include <stdint.h>
#include <stddef.h>
#include <boot/disk.h>
#include <boot/real.h>
#include <boot/alloc.h>
#include <lib/string.h>
#include <lib/misc.h>
#include <lib/e9.h>
#include <lib/vga.h>

#define BLOCK_SIZE_IN_SECTORS 16
#define BLOCK_SIZE (sector_size * BLOCK_SIZE_IN_SECTORS)

#define CACHE_INVALID (~((uint64_t)0))

#define MAX_CACHE 16384

static int cached_drive = -1;
static uint8_t *cache = NULL;
static uint64_t cached_block = CACHE_INVALID;

typedef struct dap {
    uint16_t size;
    uint16_t count;
    uint16_t offset;
    uint16_t segment;
    uint64_t lba;
} dap_t;

static struct dap *dap = NULL;

static int cache_block(int drive, uint64_t block, int sector_size) {
    if (drive == cached_drive && block == cached_block)
        return 0;

    if (!dap) {
        dap = mem_alloc(sizeof(dap_t));
        dap->size = 16;
        dap->count = BLOCK_SIZE_IN_SECTORS;
    }

    if (!cache)
        cache = mem_alloc_aligned(MAX_CACHE, 16);

    dap->segment = real_segment(cache);
    dap->offset = real_offset(cache);
    dap->lba = block * BLOCK_SIZE_IN_SECTORS;

    struct real_regs regs = {0};
    regs.eax = 0x4200;
    regs.edx = drive;
    regs.esi = (uint32_t)real_offset(dap);
    regs.ds = real_segment(dap);

    real_int(0x13, &regs, &regs);

    if (regs.eflags & EFLAGS_CF) {
        int ah = (regs.eax >> 8) & 0xff;
        e9_printf("[triaboot-stage1] Disk read error: 0x%.4x (drive number %d)\n", ah, drive);
        vga_printf("!%.4X", ah);
        while (1)
            __asm__ volatile ("hlt");
        __builtin_unreachable();
    }

    cached_block = block;
    cached_drive = drive;

    return 0;
}

int disk_get_sector_size(int drive) {
    real_regs_t regs = {0};
    drive_params_t drive_params;

    regs.eax = 0x4800;
    regs.edx = drive;
    regs.ds = real_segment(&drive_params);
    regs.esi = real_offset(&drive_params);

    drive_params.buffer_size = sizeof(drive_params_t);

    real_int(0x13, &regs, &regs);

    if (regs.eflags & EFLAGS_CF) {
        int ah = (regs.eax >> 8) & 0xff;
        e9_printf("[triaboot-stage1] Disk read error: 0x%.4x (drive number %d)\n", ah, drive);
        vga_printf("!%.4X", ah);
        while (1)
            __asm__ volatile ("hlt");
        __builtin_unreachable();
    }

    return drive_params.bytes_per_sector;
}

int disk_read_bytes(int drive, void *buffer, uint64_t loc, uint64_t count) {
    int sector_size = disk_get_sector_size(drive);

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
