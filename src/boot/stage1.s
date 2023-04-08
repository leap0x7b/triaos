section .stage1

global _start
extern stage2_begin
extern stage2_sector_size
extern stack_end
extern stage2_entry

bits 16
_start:
    jmp short entry
    nop

;%define FLOPPY
%ifdef FLOPPY ; Floppy disks have to be formatted in FAT12/16
bpb:
    .oem_name db 'triaOS  '
    .bytes_per_sector dw 512
    .sectors_per_cluster db 1
    .reserved_sectors dw 40
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
    ; BIOS method
    mov ax, 0x2401 ; L is real
    int 0x15
    jz error
    call check_a20
    jnz error
    ; Keyboard method
    call .kbd_wait
    mov al, 0xad
    out 0x64, al
    call .kbd_wait
    mov al, 0xd0
    out 0x64, al
    call .kbd_wait2
    in al, 0x60
    push ax
    call .kbd_wait
    mov al, 0xd1
    out 0x64, al
    call .kbd_wait
    pop ax
    or al, 2
    out 0x60, al
    call .kbd_wait
    mov al, 0xae
    out 0x64, al
    call .kbd_wait
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

load_stage2:
    mov bx, stage2_begin
    mov dh, stage2_sector_size
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

.code16:
    dw 0xffff
    dw 0
    db 0
    db 10011010b
    db 1
    db 0

.data16:
    dw 0xffff
    dw 0
    db 0
    db 10010010b
    db 1
    db 0

.code32:
    dw 0xffff
    dw 0
    db 0
    db 10011010b
    db 11001111b
    db 0

.data32:
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

    jmp 0x18:stage2_entry

TRIABOOT_PART1_MSG: db "tr", 0
