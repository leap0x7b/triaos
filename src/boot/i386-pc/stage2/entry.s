extern bss_start
extern bss_end
extern stack_end
extern main
global _start

_start:
    mov eax, 0x10
    mov ds, eax
    mov es, eax
    mov fs, eax
    mov gs, eax
    mov ss, eax
    mov esp, stack_end

    xor al, al
    mov edi, bss_start
    mov ecx, bss_end
    sub ecx, bss_start
    rep stosb

    call main
    jmp .halt

.halt:
    hlt
    jmp $
