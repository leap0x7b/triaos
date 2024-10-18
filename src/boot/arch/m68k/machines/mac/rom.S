.section .text

.macro rcinit
linkw %fp,#0
move.l %d2,-(%sp)
.endm

.macro rcexit
move.l (%sp)+,%d2
unlk %fp
rts
.endm

.global BiRomReadDisk
.type BiRomReadDisk, @function
BiRomReadDisk:
    rcinit
    move.l %fp@(8),%a0
    move.l #0x0, %d0
    .short 0xA002 // _Read(%a0)
    rcexit

.global BiRomSysError
.type BiRomSysError, @function
BiRomSysError:
    rcinit
    move.w %fp@(8),%d0
    .short 0xA9C9 // _SysError(%d0)
    rcexit

.global _BiRomInitGraf
.type _BiRomInitGraf, @function
_BiRomInitGraf:
    rcinit
    move.l %fp@(8),%d0
    move.l %d0,-(%sp)
    .short 0xA86E // _InitGraf
    add.l #4, %sp
    rcexit