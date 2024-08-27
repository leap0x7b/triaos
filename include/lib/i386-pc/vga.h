#ifndef LIB__I386_PC__VGA_H
#define LIB__I386_PC__VGA_H

#include <stdint.h>
#include <stddef.h>

#define VGA_AC_INDEX        0x3C0
#define VGA_AC_WRITE        0x3C0
#define VGA_AC_READ         0x3C1
#define VGA_MISC_WRITE      0x3C2
#define VGA_SEQ_INDEX       0x3C4
#define VGA_SEQ_DATA        0x3C5
#define VGA_DAC_READ_INDEX  0x3C7
#define VGA_DAC_WRITE_INDEX 0x3C8
#define VGA_DAC_DATA        0x3C9
#define VGA_MISC_READ       0x3CC
#define VGA_GC_INDEX        0x3CE
#define VGA_GC_DATA         0x3CF
//                          Color    Mono
#define VGA_CRTC_INDEX      0x3D4 /* 0x3B4 */
#define VGA_CRTC_DATA       0x3D5 /* 0x3B5 */
#define VGA_INSTAT_READ     0x3DA

#define VGA_NUM_SEQ_REGS  5
#define VGA_NUM_CRTC_REGS 25
#define VGA_NUM_GC_REGS   9
#define VGA_NUM_AC_REGS   21
#define VGA_NUM_REGS      (1 + VGA_NUM_SEQ_REGS + VGA_NUM_CRTC_REGS + \
                           VGA_NUM_GC_REGS + VGA_NUM_AC_REGS)

#define VGA_MODE_TEXT_80X25                             \
    {                                                   \
        /* MISC */                                      \
    	0x67,                                           \
        /* SEQ */                                       \
    	0x03, 0x00, 0x03, 0x00, 0x02,                   \
        /* CRTC */                                      \
    	0x5F, 0x4F, 0x50, 0x82, 0x55, 0x81, 0xBF, 0x1F, \
    	0x00, 0x4F, 0x0D, 0x0E, 0x00, 0x00, 0x00, 0x50, \
    	0x9C, 0x0E, 0x8F, 0x28, 0x1F, 0x96, 0xB9, 0xA3, \
    	0xFF,                                           \
        /* GC */                                        \
    	0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x0E, 0x00, \
    	0xFF,                                           \
        /* AC */                                        \
    	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07, \
    	0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, \
    	0x0C, 0x00, 0x0F, 0x08, 0x00                    \
    }

#define VGA_MODE_640X480X16                             \
    {                                                   \
        /* MISC */                                      \
	    0xE3,                                           \
        /* SEQ */                                       \
	    0x03, 0x01, 0x08, 0x00, 0x06,                   \
        /* CRTC */                                      \
	    0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0x0B, 0x3E, \
	    0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
	    0xEA, 0x0C, 0xDF, 0x28, 0x00, 0xE7, 0x04, 0xE3, \
	    0xFF,                                           \
        /* GC */                                        \
	    0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x05, 0x0F, \
	    0xFF,                                           \
        /* AC */                                        \
	    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07, \
	    0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, \
	    0x01, 0x00, 0x0F, 0x00, 0x00                    \
    }

#endif
