extern bss_begin
extern bss_end
extern stack_end
extern main
global stage2_entry

stage2_entry:
    mov eax, 0x20
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, stack_end

    xor al, al
    mov edi, bss_begin
    mov ecx, bss_end
    sub ecx, bss_begin
    rep stosb

    call main
    jmp .halt

.halt:
    hlt
    jmp $
