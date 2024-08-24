extern bss_start
extern bss_end
extern stack_end
extern Main
global _start

section .entry
_start:
    mov eax, 0x20
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, stack_end

    xor dh, dh
    push edx

    xor al, al
    mov edi, bss_start
    mov ecx, bss_end
    sub ecx, bss_start
    rep stosb

    call Main
    jmp .halt

.halt:
    hlt
    jmp $