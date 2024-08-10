org 0x7c00
bits 16

_start:
    jmp short entry
    nop

; keep the bpb mostly empty because it would be overwritten anyway
bpb:
    .oem_name db 'triaOS  '
    .bytes_per_sector dw 512
    .sectors_per_cluster db 0
    .reserved_sectors dw 0
    .fat_count db 0
    .root_dir_entries dw 0
    .sector_count dw 0
    .media_descriptor db 0
    .sectors_per_fat dw 0
    .sectors_per_track dw 0
    .head_count dw 0
    .hidden_sectors dd 0
    .large_sector_count dd 0
    ; FAT32 extended BPB
    .sectors_per_fat32 dd 0
    .flags dw 0
    .fat_version_number dw 0
    .root_dir_cluster dd 0
    .fsinfo_sector dw 0
    .backup_boot_sector dw 0
    .reserved0 times 12 db 0
    ; FAT12/16 extended BPB
    .drive_number db 0x80
    .reserved1 db 0
    .boot_signature db 0x29
    .serial_number dd 0x00761A05 ; "triaOS" spelled in hexadecimal
    .volume_label db 'triaOS 0.10'
    .file_system db 'FAT32   '

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
    mov [drive_number], dl
    sti

    mov al, 't'
    mov ah, 0x0E
    int 0x10

    call check_a20
    jnz enable_a20

    mov ah, 0x41
    mov bx, 0x55AA
    mov dl, [drive_number]
    int 13h
    jc error

    cmp ah, 0x80
    je error

    cmp ah, 0x86
    je error

    mov eax, [dap.lba]
    add eax, [bpb.hidden_sectors]
    mov [dap.lba], eax

    mov ah, 0x42
    mov dl, [drive_number]
    mov si, dap
    int 13h

    jnc protected_mode_switch
    jmp error

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

dap:
    .size db 0x10
    .reserved db 0
    .sector_count dw 0x20
    .address dd 0x7E00
    .lba dq 8

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

protected_mode_switch:
    cli
    lgdt [gdt.descriptor]

    mov eax, cr0
    bts ax, 0
    mov cr0, eax

    jmp 0x18:0x7E00

times 505-($-$$) db 0
dd 0x761AB007 ; triaboot signature to validate the vbr when its loaded by the bootsector
drive_number db 0
dw 0xAA55
