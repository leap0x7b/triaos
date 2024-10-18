extern bss_start
extern bss_end
extern stack_end
extern MultibootEntry
global _start

MULTIBOOT_MAGIC equ 0x1BADB002
MULTIBOOT_ALIGN_MODULES equ 1 << 0
MULTIBOOT_PROVIDE_MEMMAP equ 1 << 1
MULTIBOOT_FLAGS equ MULTIBOOT_ALIGN_MODULES | MULTIBOOT_PROVIDE_MEMMAP

section .multiboot
align 4
	.magic dd MULTIBOOT_MAGIC
	.flags dd MULTIBOOT_FLAGS
	.checksum dd -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

_start:
    mov esp, stack_end
    push ebx
    push eax

    xor al, al
    mov edi, bss_start
    mov ecx, bss_end
    sub ecx, bss_start
    rep stosb

    call MultibootEntry
    jmp .halt

.halt:
    hlt
    jmp $