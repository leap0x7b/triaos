[org 0x7c00]
[bits 16]

STAGE2_OFFSET equ 0x1000

jmp short start
nop

;%define FLOPPY
%ifdef FLOPPY ; Floppy disks have to be formatted in FAT12/16
bpb:
    .oem_name db 'triaOS  '
    .bytes_per_sector dw 512
    .sectors_per_cluster db 1
    .reserved_sectors dw 1
    .fat_count db 0x02
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
    .serial_number dd 0x761a0500 ; triaOS spelled in hexadecimal
    .volume_label db 'triaOS 0.1 '
    .file_system db 'FAT16   '
%else
bpb:
    .oem_name db 'triaOS  '
    .bytes_per_sector dw 512
    .sectors_per_cluster db 1
    .reserved_sectors dw 32
    .fat_count db 0x02
    .root_dir_entries dw 0
    .sector_count dw 0
    .media_descriptor db 0xf8
    .sectors_per_fat dw 0x09
    .sectors_per_track dw 0x12
    .head_count dw 0x02
    .hidden_sectors dd 0
    .large_sector_count dd 0
    ; FAT32 extended BPB
    .sectors_per_fat32 dd 0
    .flags dw 0
    .fat_version_number dw 0
    .root_dir_cluster dd 0x02
    .fsinfo_sector dw 0
    .backup_boot_sector dw 0
    .reserved0 times 12 db 0
    ; FAT12/16 extended BPB
    .drive_number db 0x80
    .reserved1 db 0
    .boot_signature db 0x29
    .serial_number dd 0x761a0500 ; triaOS spelled in hexadecimal
    .volume_label db 'triaOS 0.1 '
    .file_system db 'FAT32   '
%endif

start:
    mov [BOOT_DRIVE], dl

    mov bp, 0x9000
    mov sp, bp

    mov bx, TRIABOOT_PART1_MSG
    call print

    call enable_a20
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

    mov cx, 0

.loop:
    cmp cx, 4
    je .end
    
    mov ax, dx
    and ax, 0x000f
    add al, 0x30
    cmp al, 0x39
    jle .step2
    add al, 7

.step2:
    mov bx, HEX_OUT + 3
    sub bx, cx
    mov [bx], al
    ror dx, 4

    add cx, 1
    jmp .loop

.end:
    mov bx, HEX_OUT
    call print

    popa
    ret

HEX_OUT db '0000', 0

error:
    mov dh, ah
    call print_hex
    jmp .loop

.loop:
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
    mov ax, 0x2401
    int 0x15
    jnz .done

.done:
    call check_a20
    jnz error
    ret

load_stage2:
    mov bx, STAGE2_OFFSET
    mov dh, 2
    mov dl, [BOOT_DRIVE]
    call disk_load
    ret

disk_load:
    pusha
    push dx

    mov ah, 0x02
    mov al, dh
    mov cl, 0x02
    mov ch, 0x00
    mov dh, 0x00

    int 0x13
    jc error

    pop dx
    cmp al, dh
    jne error

    popa
    ret

gdt:
    dq 0x0

.code:
    dw 0xffff
    dw 0x0
    db 0x0
    db 10011010b
    db 11001111b
    db 0x0

.data:
    dw 0xffff
    dw 0x0       ; segment base, bits 0-15
    db 0x0       ; segment base, bits 16-23
    db 10010010b ; flags (8 bits)
    db 11001111b ; flags (4 bits) + segment length, bits 16-19
    db 0x0       ; segment base, bits 24-31

.end:

.descriptor:
    dw gdt.end - gdt - 1
    dd gdt

CODE_SEG equ gdt.code - gdt
DATA_SEG equ gdt.data - gdt

protected_mode_switch:
    cli
    lgdt [gdt.descriptor]
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    jmp CODE_SEG:.init

[bits 32]
.init:
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov ebp, 0x90000
    mov esp, ebp

    mov esi, TRIABOOT_PART2_MSG
    call print32

    call .start

.start:
    call STAGE2_OFFSET
    jmp $

get_cursor32:
    push eax
    push edx
    mov ebx, 0
    mov dx, 0x3D4
    mov al, 0x0F
    out dx, al
    mov dx, 0x3D5
    in al, dx
    mov bl, al
    mov dx, 0x3D4
    mov al, 0x0E
    out dx, al
    mov dx, 0x3D5
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

    mov ebx, 0
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
