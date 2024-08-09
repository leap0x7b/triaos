/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include <lib/fatfs/ff.h>			/* Obtains integer types */
#include <lib/fatfs/diskio.h>		/* Declarations of disk functions */

#include <boot/i386-pc/disk.h>
#include <lib/e9.h>

/* Definitions of physical drive number for each drive */
#define DEV_FLOPPY 0
#define DEV_HDD 1

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
    BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
    DSTATUS stat;
    int result;

    switch (pdrv) {
    case DEV_FLOPPY:
    case DEV_HDD:
        return RES_OK;
    }

    return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
    BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
    DSTATUS stat;
    int result;

    switch (pdrv) {
    case DEV_FLOPPY:
    case DEV_HDD:
        return RES_OK;
    }

    return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
    BYTE pdrv,		/* Physical drive nmuber to identify the drive */
    BYTE *buff,		/* Data buffer to store read data */
    LBA_t sector,	/* Start sector in LBA */
    UINT count		/* Number of sectors to read */
)
{
    DRESULT res;
    int result;

    switch (pdrv) {
    case DEV_FLOPPY:
        if (!buff)
            return RES_PARERR;

        TiE9Printf("buf addr: 0x%x, sector: %d, count: %d\n", buff, sector, count);
        BiDiskReadBytes(0, buff, sector, count * BiDiskGetSectorSize(0));

        return RES_OK;

    case DEV_HDD:
        if (!buff)
            return RES_PARERR;

        TiE9Printf("buf addr: 0x%x, sector: %d, count: %d\n", buff, sector, count);
        BiDiskReadBytes(0, buff, sector, count * BiDiskGetSectorSize(0x80));

        return RES_OK;
    }

    return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
    BYTE pdrv,			/* Physical drive nmuber to identify the drive */
    const BYTE *buff,	/* Data to be written */
    LBA_t sector,		/* Start sector in LBA */
    UINT count			/* Number of sectors to write */
)
{
    DRESULT res;
    int result;

    switch (pdrv) {
    case DEV_FLOPPY:
    case DEV_HDD:
        res = RES_NOTRDY;

        return res;
    }

    return RES_PARERR;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
    BYTE pdrv,		/* Physical drive nmuber (0..) */
    BYTE cmd,		/* Control code */
    void *buff		/* Buffer to send/receive control data */
)
{
    DRESULT res;
    int result;

    switch (pdrv) {
    case DEV_FLOPPY:
    case DEV_HDD:
        switch (cmd) {
        case CTRL_SYNC:
            res = RES_OK;
            break;
        case GET_SECTOR_COUNT:
            *(DWORD*) buff = BiDiskGetSectorSize(pdrv == DEV_HDD ? 0x80 : 0);
            res = RES_OK;
            break;
        case GET_BLOCK_SIZE:
            *(DWORD*) buff = 16;
            res = RES_OK;
            break;
        }
        return RES_PARERR;
    }

    return RES_PARERR;
}
