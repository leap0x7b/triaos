SECTIONS {
    . = 0x7e00;

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
    . += 0x1000;
    stack_end = .;
}
