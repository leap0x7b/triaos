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
    e9_write("[triaboot-stage2] Floppy MBR content:\n");
    char buf[512];
    disk_read_bytes(0x80, buf, 0, 512);
    for (int i = 0; i < 512; i++)
        e9_write_char((buf[i] >= ' ' && buf[i] <= '~') ? buf[i] : '.');
    e9_write_char('\n');
    FATFS fs;
    FRESULT result = f_mount(&fs, "1:", 1);
    if (result != FR_OK)
        e9_printf("[triaboot-stage2] FATFS Error: %d\n", result);
    FIL readme_file;
    char readme[1345];
    result = f_open(&readme_file, "0:/README.md", FA_READ);
    if (result != FR_OK)
        e9_printf("[triaboot-stage2] FATFS Error: %d\n", result);
    uint32_t readme_bytes_read;
    f_read(&readme_file, &readme, 1345, &readme_bytes_read);
    vga_write(readme);
}
