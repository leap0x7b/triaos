OUTPUT_FORMAT(elf32-i386)

SECTIONS {
    . = 0x100000;

    .text : {
        *(.text*)
    }

    .rodata : {
        *(.rodata*)
    }

    .data : {
        *(.data)
    }

    .bss : {
        *(COMMON)
        *(.bss)
    }

    /DISCARD/ : {
        *(.eh_frame);
        *(.comment);
    }
}
