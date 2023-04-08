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

#include <boot/disk.h>

/* Definitions of physical drive number for each drive */
#define DEV_FLOPPY 0
#define DEV_HDD 1

//extern uint8_t temporary_sector_buffer[SECTOR_SIZE];

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

        disk_read(0, buff, sector * 512, count * 512);

        return RES_OK;

    case DEV_HDD:
        if (!buff)
            return RES_PARERR;

        disk_read(0x80, buff, sector * 512, count * 512);

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
        res = RES_OK;

        return res;
    }

    return RES_PARERR;
}
