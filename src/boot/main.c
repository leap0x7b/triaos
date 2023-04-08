#define DBG
#include <lib/vga.h>
#include <lib/e9.h>
#include <lib/printf.h>
#include <lib/fatfs/ff.h>
#include <boot/e820.h>
#include <boot/alloc.h>
#include <boot/disk.h>

void main(void) {
    e9_write("[triaboot-stage2] Hello World!\n");
    vga_init();
    vga_write("ia");
    e9_write("[triaboot-stage2] E820 memory map: ");
    e820_init();
    e9_write("OK\n");
    FATFS fs;
    FRESULT result = f_mount(&fs, "0:", 0);
    if (result != FR_OK)
        e9_printf("[triaboot-stage2] FATFS Error: %d\n", result);
}
