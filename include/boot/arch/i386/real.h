// Adapted from Limine (https://github.com/limine-bootloader/limine/blob/trunk/common/lib/real.s2.asm_bios_ia32)
// Originally licensed under BSD-2-Clause

#ifndef BOOT__ARCH__I386__REAL_H
#define BOOT__ARCH__I386__REAL_H

#include <stdint.h>

#define BiRealSegment(x) ((uint16_t)(((int)x & 0xFFFF0) >> 4))
#define BiRealOffset(x) ((uint16_t)(((int)x & 0x0000F) >> 0))

#define BiRealDesegment(seg, off) (((uint32_t)(seg) << 4) + (uint32_t)(off))

#define EFLAGS_CF (1 << 0)
#define EFLAGS_ZF (1 << 6)

typedef struct {
    uint16_t gs;
    uint16_t fs;
    uint16_t es;
    uint16_t ds;

    uint32_t eflags;

    uint32_t ebp;
    uint32_t edi;
    uint32_t esi;
    uint32_t edx;
    uint32_t ecx;
    uint32_t ebx;
    uint32_t eax;
} __attribute__((packed)) BiRealRegisters;

void __attribute__((cdecl)) BiRealInterrupt(uint8_t int_num, BiRealRegisters *out_regs, BiRealRegisters *in_regs);

#endif
