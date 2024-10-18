#include <lib/console.h>
#include <lib/arch/i386/machines/pc/e9.h>
#include <boot/file.h>
#include <boot/arch/i386/machines/pc/disk.h>
#include <boot/arch/i386/machines/pc/partition.h>
#include <boot/arch/i386/machines/pc/e820.h>
#include <boot/fs/fat32.h>

void Main(int boot_drive) {
    TiE9Write("triaBoot 0.10\n");
    TiConsoleInit();
    TiConsoleWriteChar('r');

    TiE9Write("E820 memory map: ");
    BiE820Init();
    TiE9Write("OK\n");

    TiE9Write("Searching for kernel...\n");
    BiPartition parts[4];
    FILE stage3;
    for (int i = 0; i < 4; i++) {
        TiE9Printf("Partition %d...", i);
        int ret = BiGetPartition(&parts[i], boot_drive, i);
        if (!ret) {
            if (!fopen(&stage3, &parts[i], "TriaOS/triakrnl.trx")) {
                TiE9Write(" found\n");
                goto found;
            } else {
                TiE9Write(" not found");
            }
        } else {
            TiE9Write(" not found");
        }
        TiE9WriteChar('\n');
    }

    while (1) {
        __asm__ ("hlt");
    }

found:;
    TiE9Write("First 512 bytes of the kernel:\n");
    char buf[512];
    fread(&stage3, buf, 0, 512);
    for (int i = 0; i < 512; i++)
        TiE9WriteChar((buf[i] >= ' ' && buf[i] <= '~') ? buf[i] : '.');
    TiE9WriteChar('\n');

    while (1) {
        __asm__ ("hlt");
    }
}
