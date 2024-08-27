SECTIONS {
    . = 0x200000;

    .text : {
        *(.multiboot*)
        *(.text*)
    }

    . += CONSTANT(MAXPAGESIZE);

    .rodata : {
        *(.rodata*)
    }

    . += CONSTANT(MAXPAGESIZE);

    .data : {
        *(.data)
    }

    .bss : {
        bss_start = .;
        *(COMMON)
        *(.bss*)
        bss_end = .;
    }

    /DISCARD/ : {
        *(.eh_frame*);
        *(.comment*);
        *(.note*);
    }

    stack_start = .;
    . += 0x8000;
    stack_end = .;
}
