OUTPUT_FORMAT(elf32-i386)

SECTIONS {
    . = 0x7c00;

    .stage1 : {
        KEEP(*(.stage1*))
        . = 0x7dfe;
        SHORT(0xaa55)
    }

    . = 0x7e00;
    stage2_begin = .;

    .text : {
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
        *(.eh_frame*);
        *(.comment*);
        *(.note*);
    }

    stage2_end = .;
    stage2_size = stage2_end - stage2_begin;
    stage2_sector_size = (stage2_size / 512) + 1;

    stack_start = .;
    . += 0x1000;
    stack_end = .;
}
