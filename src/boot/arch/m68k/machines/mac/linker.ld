PHYS_START = 0x10000;
PHYS_LEN = 12288;

MEMORY {
    ram (rwx) : org = PHYS_START, l = PHYS_LEN
}

SECTIONS {
    . = PHYS_START;
    stage2_start = .;

    .text : ALIGN(2) {
        *(.stage1*)
        *(.text*)
    } >ram

    .data : ALIGN(2) {
        *(.data*)
    } >ram

    .rodata : ALIGN(2) {
        *(.rodata*)
    } >ram

    .bss : ALIGN(2) {
        bss_start = .;
        *(COMMON)
        *(.bss*)
        bss_end = .;
    } >ram
    
    heap_begin = .;

    . = PHYS_START + PHYS_LEN;

    stage2_end = .;
}