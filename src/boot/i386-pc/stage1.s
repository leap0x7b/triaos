org 0x7c00
bits 16

_start:
    jmp short entry
    nop

;%define TRIAFS
%ifdef TRIAFS
bpb:
    .magic db 'TRIAFS', 0x7f
    .version db 1
    .bytes_per_block dw 512
    .block_count db 0
    .root_dir_block_count dw 0
    .drive_number db 0x80
    .serial_number dd 0x54524941 ; "TRIA" in hexadecimal
    .volume_label db 'triaOS0.1.0'
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
    .sectors_per_fat dw 0
    .sectors_per_track dw 32
    .head_count dw 64
    .hidden_sectors dd 0
    .large_sector_count dd 0x00020000
    ; FAT32 extended BPB
    .sectors_per_fat32 dd 0x000003f1
    .flags dw 0
    .fat_version_number dw 0
    .root_dir_cluster dd 2
    .fsinfo_sector dw 1
    .backup_boot_sector dw 6
    .reserved0 times 12 db 0
    ; FAT12/16 extended BPB
    .drive_number db 0x80
    .reserved1 db 0
    .boot_signature db 0x29
    .serial_number dd 0x54524941 ; "TRIA" in hexadecimal
    .volume_label db 'triaOS0.1.0'
    .file_system db 'FAT32   '
%endif

entry:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00
    mov [bpb.drive_number], dl

    mov si, TRIABOOT_PART1_MSG
    call print

    call check_a20
    jnz enable_a20

	mov ax, [bpb.sectors_per_fat]

	mov bx, 2
	mul bx
	xor bx, bx

	add ax, [bpb.reserved_sectors]
	add ax, [bpb.hidden_sectors]

	mov [dap.lba], ax
	mov word [dap.sector_count], 1
	mov bx, 0x7e00
	mov word [dap.offset], 0x7e00
	call disk_load

	add bx, 0x1a
	mov ax, word [bx]
	call cluster_to_lba
	mov [dap.lba], ax

	mov bx, 0x7e00
	add bx, 28
	mov eax, dword [bx]
	mov bx, 512
	div bx

	cmp dx, 0
	je .part2
	add ax, 1
.part2:
	mov dl, 0x80
	mov word [dap.sector_count], ax
	call disk_load

    call protected_mode_switch

    jmp $

print:
    mov ah, 0x0e

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
    and ax, 0x0f

    mov bx, HEX_OUT
    add bx, cx

    cmp ax, 0x0a
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
    mov si, HEX_OUT
    call print

    popa
    ret

HEX_OUT db "0000", 0

error:
    mov dh, ah
    mov al, '!'
    mov ah, 0x0e
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

dap:
	.size db 0x10
	.reserved db 0
    .sector_count dw 28
    .offset dw 0x7e00
    .segment dw 0
    .lba dq 0

disk_load:
    pusha

	mov dl, 0x80
	mov ah, 0x42
	mov si, dap
	int 13h

	mov ah, 0xe
    jc error

    popa
    ret

cluster_to_lba:
	sub ax, 2
	xor bx, bx
	mov bl, byte [bpb.sectors_per_cluster]
	mul bx
	add ax, word [bpb.reserved_sectors]
	add ax, [bpb.sectors_per_fat32]
	add ax, [bpb.sectors_per_fat32]
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

    jmp 0x18:0x7e00

TRIABOOT_PART1_MSG: db "tr", 0
TRIABOOT_FILENAME: db "TRIABOOT S2"

times 510-($-$$) db 0x00
dw 0xaa55
