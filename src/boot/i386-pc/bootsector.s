BITS 16           ; We are in 16-bit real mode
ORG 0x7C00        ; The boot sector is loaded at 0x7C00

_start:
    cli
    cld
    jmp 0:.part2
.part2:
    ; Initialize the segment registers
    xor ax, ax      ; AX = 0x0000
    mov ds, ax      ; DS = 0x0000
    mov es, ax      ; ES = 0x0000
    mov ss, ax      ; SS = 0x0000
    mov sp, 0x7C00  ; Set up the stack pointer
    mov [drive_number], dl

    mov al, 't'
    mov ah, 0x0E
    int 0x10

    ; Check for a bootable partition
    mov si, 0x1BE   ; Start of partition table entries
    mov cx, 4       ; Number of partition entries to check

find_bootable_partition:
    mov ax, [si + 4]  ; Load the partition type (byte at offset 4)
    test ax, ax     ; Check if the partition type is non-zero
    jnz load_vbr    ; If it is non-zero, jump to load VBR

    add si, 16      ; Move to the next partition entry
    loop find_bootable_partition ; Repeat for all 4 entries

    ; If no bootable partition was found, hang
    jmp $

load_vbr:
    ; Load the starting sector of the partition
    mov dx, [si + 8]  ; Load the lower word of LBA address
    mov cx, [si + 10] ; Load the upper word of LBA address
    push cx
    push dx

    ; Load the VBR into 0000:7E00
    mov bx, 0x7E00  ; Destination address in ES:BX
    mov ax, 0x0201  ; BIOS read sectors function (AH=2), 1 sector (AL=1)
    int 0x13        ; BIOS interrupt

    ; Check for the magic identifier 0x761A before 0xAA55
    mov si, 0x7E00  ; SI points to the start of VBR
    add si, 510     ; Move SI to the magic identifier position
    mov ax, [si - 2]  ; Load the word at [SI-2]
    cmp ax, 0x761A  ; Check if it's the magic identifier
    jne error  ; If not, display error

    ; Jump to the VBR code
    jmp 0x0000:0x7E00

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

; Boot signature (must be at the end of the sector)
times 509-($-$$) db 0 ; Pad remaining bytes with zeros
drive_number db 0
dw 0xAA55             ; Boot signature
