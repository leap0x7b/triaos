org 0x7C00
bits 16

_start:
    jmp short entry
    nop

bpb: times 0x5A-($-$$) db 0

entry:
    cli
    cld
    jmp 0:.part2
.part2:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti

    mov al, 't'
    mov ah, 0x0E
    int 0x10

    mov eax, dword [stage2_sector]
    mov ebx, 0x7E00
    mov ecx, 1
    call read_sectors

    jc error
    jmp 0x7E00

print:
    mov ah, 0x0E
.loop:
    lodsb
    cmp al, 0
    je .done
    int 0x10
    jmp .loop
.done:
    ret

print_hex:
    pusha
    mov cx, 4
.loop:
    dec cx

    mov ax, dx
    shr dx, 4
    and ax, 0x0F

    mov bx, .out
    add bx, cx

    cmp ax, 0x0A
    jl .step2

    add al, 7
    jl .step2
.step2:
    add al, 0x30
    mov byte [bx], al

    cmp cx, 0
    je .done
    jmp .loop
.done:
    mov si, .out
    call print

    popa
    ret
.out db "0000", 0

error:
    mov dh, ah
    mov al, '!'
    mov ah, 0x0E
    int 0x10

    call print_hex
    jmp .halt
.halt:
    hlt
    jmp $

read_sector:
    push eax
    push ebx
    push ecx
    push edx
    push esi
    push edi

    push es
    pop word [dap.segment]
    mov word [dap.offset], bx
    mov dword [dap.lba_low], eax

    xor esi, esi
    mov si, dap
    mov ah, 0x42

    clc
    int 0x13

    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx
    pop eax
    ret

dap:
    .size db 0x10
    .reserved db 0
    .sector_count dw 1
    .offset dw 0
    .segment dw 0
    .lba_low dd 0
    .lba_high dd 0

%define READ_BUFFER 0x7000
%define BYTES_PER_SECTOR 512

read_sectors:
    push eax
    push ebx
    push ecx
    push edx
    push esi
    push edi
.loop:
    push es
    push ebx

    mov bx, READ_BUFFER
    mov es, bx
    xor bx, bx

    call read_sector

    pop ebx
    pop es

    jc .done

    push ds

    mov si, READ_BUFFER
    mov ds, si
    mov edi, ebx
    xor esi, esi

    push ecx
    mov ecx, BYTES_PER_SECTOR
    a32 o32 rep movsb
    pop ecx

    pop ds

    inc eax
    add ebx, BYTES_PER_SECTOR

    loop .loop
.done:
    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx
    pop eax
    ret

times 0x1B0-($-$$) db 0
stage2_sector: dd 1

times 0x1B8-($-$$) db 0
times 510-($-$$) db 0
dw 0xaa55

stage2:
    call check_a20
    jnz enable_a20

    mov eax, dword [stage2_sector]
    inc eax
    mov ebx, 0x8000
    mov ecx, 63
    call read_sectors
    jc error

    cli
    lgdt [gdt.descriptor]

    mov eax, cr0
    bts ax, 0
    mov cr0, eax

    jmp 0x18:0x8000

gdt:
    dq 0
.code16:
    dw 0xFFFF
    dw 0
    db 0
    db 10011010b
    db 1
    db 0
.data16:
    dw 0xFFFF
    dw 0
    db 0
    db 10010010b
    db 1
    db 0
.code32:
    dw 0xFFFF
    dw 0
    db 0
    db 10011010b
    db 11001111b
    db 0
.data32:
    dw 0xFFFF
    dw 0
    db 0
    db 10010010b
    db 11001111b
    db 0
.end:
.descriptor:
    dw (.end - gdt) - 1
    dd gdt

check_a20:
    push es
    xor ax, ax
    dec ax
    mov es, ax
    mov ah, byte [es:0x510]
    mov byte [ds:0x500], 0
    mov byte [es:0x510], al
    mov al, byte [ds:0x500]
    mov byte [es:0x510], ah
    pop es
    or al, al
    ret

enable_a20:
    ; BIOS method
    mov ax, 0x2401 ; L is real
    int 0x15
    jz error
    call check_a20
    jnz error

    ; Keyboard method
    call .kbd_wait
    mov al, 0xAD
    out 0x64, al
    call .kbd_wait
    mov al, 0xD0
    out 0x64, al
    call .kbd_wait2
    in al, 0x60
    push ax
    call .kbd_wait
    mov al, 0xD1
    out 0x64, al
    call .kbd_wait
    pop ax
    or al, 2
    out 0x60, al
    call .kbd_wait
    mov al, 0xAE
    out 0x64, al
    call .kbd_wait
    call check_a20
    jnz error

    ; Fast A20 method
    in al, 0x92
    or al, 2
    jz error
    or al, 2
    and al, 0xFE
    out 0x92, al
    call check_a20
    jnz error
    ret
.kbd_wait:
    in al, 0x64
    test al, 2
    jnz .kbd_wait
    ret
.kbd_wait2:
    in al, 0x64
    test al, 1
    jz .kbd_wait2
    ret

times 1024-($-$$) db 0
incbin "triaboot.s2"
times 32768-($-$$) db 0