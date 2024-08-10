#include <lib/printf.h>
#include <lib/i386-pc/vga.h>
#include <lib/i386-pc/e9.h>
#include <boot/alloc.h>
#include <boot/i386-pc/e820.h>
#include <boot/i386-pc/disk.h>
#include <boot/i386-pc/fatfs/ff.h>

void Main(void) {
    TiE9Write("[triaboot-stage3] Hello World!\n");
    TiVgaInit();
    TiVgaWrite("i");
    TiE9Write("[triaboot-stage3] E820 memory map: ");
    BiE820Init();
    TiE9Write("OK\n");
    TiE9Write("[triaboot-stage3] Floppy MBR content:\n");
    char buf[512];
    BiDiskReadBytes(0x80, buf, 0, 512);
    for (int i = 0; i < 512; i++)
        TiE9WriteChar((buf[i] >= ' ' && buf[i] <= '~') ? buf[i] : '.');
    TiE9WriteChar('\n');
    FATFS fs;
    FRESULT result = f_mount(&fs, "1:", 1);
    if (result != FR_OK)
        TiE9Printf("[triaboot-stage3] FATFS Error: %d\n", result);
    FIL readme_file;
    char readme[1345];
    result = f_open(&readme_file, "0:/README.md", FA_READ);
    if (result != FR_OK)
        TiE9Printf("[triaboot-stage3] FATFS Error: %d\n", result);
    uint32_t readme_bytes_read;
    f_read(&readme_file, &readme, 1345, &readme_bytes_read);
    TiVgaWrite(readme);

    while (1) {
        __asm__ ("hlt");
    }
}
