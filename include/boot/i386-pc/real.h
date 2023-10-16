// Adapted from Limine (https://github.com/limine-bootloader/limine/blob/trunk/common/lib/real.s2.asm_bios_ia32)
// Originally licensed under BSD-2-Clause

#ifndef BOOT__REAL_H
#define BOOT__REAL_H

#include <stdint.h>

#define real_segment(x) ((uint16_t)(((int)x & 0xffff0) >> 4))
#define real_offset(x) ((uint16_t)(((int)x & 0x0000f) >> 0))

#define real_desegment(seg, off) (((uint32_t)(seg) << 4) + (uint32_t)(off))

#define EFLAGS_CF (1 << 0)
#define EFLAGS_ZF (1 << 6)

typedef struct real_regs {
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
} __attribute__((packed)) real_regs_t;

void __attribute__((cdecl)) real_int(uint8_t int_num, real_regs_t *out_regs, real_regs_t *in_regs);

#endif
