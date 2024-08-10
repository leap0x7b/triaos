SECTIONS {
    . = 0x7E00;

    .entry : {
        *(.start*)
    }

    .text : {
        *(.text*)
    }

    .data : {
        *(.data*)
    }

    .rodata : {
        *(.rodata*)
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
