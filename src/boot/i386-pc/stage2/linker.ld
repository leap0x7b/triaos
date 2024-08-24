SECTIONS {
    . = 0x8000;

    .entry : {
        *(.start*)
    }

    .real : {
        *(.real*)
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
    . += 0x8000;
    stack_end = .;
}
