OUTPUT_FORMAT(elf32-i386)

SECTIONS {
    . = 0x8000;

    .text : {
        KEEP(*(.entry*))
        *(.text*)
    }

    .rodata : {
        *(.rodata*)
    }

    .data : {
        *(.data*)
    }

    .bss : {
        bss_begin = .;
        *(COMMON)
        *(.bss*)
        bss_end = .;
    }

    /DISCARD/ : {
        *(.eh_frame);
        *(.comment);
    }
}
