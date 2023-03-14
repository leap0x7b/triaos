[org 0x7c00]
[bits 16]

_start:
    jmp short entry
    nop

;%define FLOPPY
%ifdef FLOPPY ; Floppy disks have to be formatted in FAT12/16
bpb:
    .oem_name db 'triaOS  '
    .bytes_per_sector dw 512
    .sectors_per_cluster db 1
    .reserved_sectors dw 1
    .fat_count db 2
    .root_dir_entries dw 0xe0
    .sector_count dw 2880
    .media_descriptor db 0xf0
    .sectors_per_fat dw 9
    .sectors_per_track dw 18
    .head_count dw 2
    .hidden_sectors dd 0
    .large_sector_count dd 0
    ; FAT12/16 extended BPB
    .drive_number db 0
    .reserved db 0
    .boot_signature db 0x29
    .serial_number dd 0x761a05 ; triaOS spelled in hexadecimal
    .volume_label db 'triaOS0.1.0'
    .file_system db 'FAT16   '
%else
bpb:
    .oem_name db 'triaOS  '
    .bytes_per_sector dw 512
    .sectors_per_cluster db 1
    .reserved_sectors dw 32
    .fat_count db 2
    .root_dir_entries dw 0
    .sector_count dw 0
    .media_descriptor db 0xf8
    .sectors_per_fat dw 9
    .sectors_per_track dw 18
    .head_count dw 2
    .hidden_sectors dd 0
    .large_sector_count dd 0
    ; FAT32 extended BPB
    .sectors_per_fat32 dd 0
    .flags dw 0
    .fat_version_number dw 0
    .root_dir_cluster dd 2
    .fsinfo_sector dw 0
    .backup_boot_sector dw 0
    .reserved0 times 12 db 0
    ; FAT12/16 extended BPB
    .drive_number db 0x80
    .reserved1 db 0
    .boot_signature db 0x29
    .serial_number dd 0x761a05 ; triaOS spelled in hexadecimal
    .volume_label db 'triaOS0.1.0'
    .file_system db 'FAT32   '
%endif

entry:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00

    mov bp, 0x9000
    mov sp, bp

    mov bx, TRIABOOT_PART1_MSG
    call print

    call check_a20
    jnz enable_a20

    call load_stage2
    call protected_mode_switch

    jmp $

print:
    pusha

.loop:
    mov al, [bx]
    cmp al, 0
    je .done

    mov ah, 0x0e
    int 0x10

    add bx, 1
    jmp .loop

.done:
    popa
    ret

print_hex:
    pusha
    mov cx, 4

.loop:
    dec cx

    mov ax, dx
    shr dx, 4
    and ax, 0x0f

    mov bx, HEX_OUT
    add bx, cx

    cmp ax, 0x0a
    jl .step2

    add al, 0x27
    jl .step2

.step2:
    add al, 0x30
    mov byte [bx], al

    cmp cx, 0
    je .done

    jmp .loop

.done:
    mov bx, HEX_OUT
    call print

    popa
    ret

HEX_OUT db "0000", 0

error:
    mov dh, ah
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
    mov ax, 0x2401 ; L is real
    int 0x15
    jz error
    ret

load_stage2:
    mov bx, stage2.offset
    mov dh, (stage2.size / 512) + 1
    call disk_load
    ret

disk_load:
    pusha
    push dx

    mov ah, 2
    mov al, dh
    mov cl, 2
    xor ch, ch
    xor dh, dh

    int 0x13
    jc error

    pop dx
    cmp al, dh
    jne error

    popa
    ret

gdt:
    dq 0

.code:
    dw 0xffff
    dw 0
    db 0
    db 10011010b
    db 11001111b
    db 0

.data:
    dw 0xffff
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
    lgdt [gdt.descriptor]
    cli

    mov eax, cr0
    bts ax, 0
    mov cr0, eax

    jmp 0x08:.init

[bits 32]
.init:
    mov eax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov ebp, 0x90000
    mov esp, ebp

    mov esi, TRIABOOT_PART2_MSG
    call print32

    call .start

.start:
    call stage2.offset
    jmp $

get_cursor32:
    push eax
    push edx

    xor ebx, ebx
    mov dx, 0x3d4
    mov al, 0x0f
    out dx, al
    mov dx, 0x3d5
    in al, dx

    mov bl, al
    mov dx, 0x3d4
    mov al, 0x0e
    out dx, al
    mov dx, 0x3d5
    in al, dx

    mov bh, al
    shl ebx, 1

    pop edx
    pop eax
    ret

set_cursor32:
    push eax
    push edx

    shr bx, 1
    mov dx, 0x3d4
    mov al, 0x0f
    out dx, al
    mov dx, 0x3d5
    mov al, bl
    out dx, al

    mov dx, 0x3d4
    mov al, 0x0e
    out dx, al
    mov dx, 0x3d5
    mov al, bh
    out dx, al

    pop edx
    pop eax
    ret

print32:
    pusha

    xor ebx, ebx
    mov edx, 0xb8000
    call get_cursor32

    add edx, ebx

.loop:
    lodsb

    mov ebx, edx
    sub ebx, 0xb8000
    call set_cursor32
    
    cmp al, 0
    je .done
    
    mov ah, 0x07
    mov [edx], ax
    add edx, 2
    
    jmp .loop

.done:
    popa
    ret

BOOT_DRIVE db 0
TRIABOOT_PART1_MSG: db "tr", 0
TRIABOOT_PART2_MSG: db "ia", 0

times 510 - ($-$$) db 0
dw 0xaa55

stage2: incbin "build/boot/stage2.bin"
.offset equ 0x8000
.size equ $ - stage2
