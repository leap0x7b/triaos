#include <lib/console.h>
#include <boot/boot_info.h>
#include <boot/multiboot.h>
#include <boot/i386-pc/e820.h>

BeMemmapEntry *memmap[E820_MAX_ENTRIES];
size_t memmap_entries = 0;

void Main(BeBootInfo *info);

void MultibootEntry(uint32_t magic, BeMultibootInfo *info) {
    TiConsoleInit();
    if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        TiConsoleWrite("Error: Not a valid Multiboot bootloader");
    }

    BeBootInfo boot_info;

    for (BeMultibootMemmapEntry *mmap = (BeMultibootMemmapEntry *)info->memmap_address; (uint64_t)mmap < info->memmap_address + info->memmap_size; mmap = (BeMultibootMemmapEntry *)((uint32_t)mmap + mmap->size + sizeof(mmap->size))) {
        BeMemmapEntry entry;
        entry.address = mmap->address;
        entry.size = mmap->size;
        entry.type = mmap->type;

        memmap[memmap_entries++] = &entry;
    }

    boot_info.magic = TRIABOOT_MAGIC;
    boot_info.command_line = info->command_line;
    boot_info.bootloader_name = info->bootloader_name;
    boot_info.memmap.entries = memmap;
    boot_info.memmap.entry_count = memmap_entries;
    
    Main(&boot_info);
}
