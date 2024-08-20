#include <lib/i386-pc/console.h>
#include <boot/boot_info.h>

void Main(BeBootInfo *info) {
    TiConsoleInit();
    if (info->magic != TRIABOOT_MAGIC) {
        TiConsoleWrite("Error: Not a valid TriaBoot-compatible bootloader");
    }
    TiConsoleWrite("TriaOS version 0.10\n");
    TiConsolePrintf("Bootloader: %s\n", info->bootloader_name);
    TiConsolePrintf("Command line arguments: %s\n", info->command_line);
}
