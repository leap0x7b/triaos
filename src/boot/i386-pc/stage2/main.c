#include <lib/i386-pc/console.h>
#include <lib/i386-pc/e9.h>
#include <boot/file.h>
#include <boot/i386-pc/disk.h>
#include <boot/i386-pc/partition.h>

void Main(void) {
    TiE9Write("[triaboot-stage2] Hello World!\n");
    TiConsoleInit();
    TiConsoleWriteChar('r');

    int boot_drive = *(uint8_t*)(0x7E00 - 3);
    BiPartition partition;
    if (BiGetPartition(&partition, boot_drive, 0)) {
        TiE9Write("[triaboot-stage2] partittion fail\n");
    }

    FILE stage3;
    if (fopen(&stage3, &partition, "triaboot.s3")) {
        TiE9Write("[triaboot-stage2] open failure\n");
    }

    while (1) {
        __asm__ ("hlt");
    }
}
