global rm_int
rm_int:
    mov al, byte [esp + 4]
    mov byte [.int_num], al

    mov eax, dword [esp + 8]
    mov dword [.out_regs], eax

    mov eax, dword [esp + 12]
    mov dword [.in_regs], eax

    sgdt [.gdt]
    sidt [.idt]
    lidt [.real_idt]

    push ebx
    push esi
    push edi
    push ebp

    jmp 0x08:.bits16

[bits 16]
.bits16:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov eax, cr0
    and al, 0xfe
    mov cr0, eax
    jmp 0x00:.cszero

.cszero:
    xor ax, ax
    mov ss, ax

    mov dword [ss:.esp], esp
    mov esp, dword [ss:.in_regs]
    pop gs
    pop fs
    pop es
    pop ds
    popfd
    pop ebp
    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx
    pop eax
    mov esp, dword [ss:.esp]

    sti

    db 0xcd

.int_num:
    db 0

    cli

    mov dword [ss:.esp], esp
    mov esp, dword [ss:.out_regs]
    lea esp, [esp + 10 * 4]
    push eax
    push ebx
    push ecx
    push edx
    push esi
    push edi
    push ebp
    pushfd
    push ds
    push es
    push fs
    push gs
    mov esp, dword [ss:.esp]

    o32 lgdt [ss:.gdt]
    o32 lidt [ss:.idt]

    mov eax, cr0
    or al, 1
    mov cr0, eax
    jmp 0x18:.bits32

[bits 32]
.bits32:
    mov ax, 0x20
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    pop ebp
    pop edi
    pop esi
    pop ebx

    ret

align 16
.esp dd 0
.out_regs dd 0
.in_regs dd 0
.gdt dq 0
.idt dq 0
.real_idt:
    dw 0x3ff
    dd 0
