#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "ff.h"     /* Obtains integer types */
#include "diskio.h" /* Declarations of disk functions */
#include <stdio.h>

spi_inst_t *sd = spi1;

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

static DSTATUS sdcard_status = STA_NOINIT;

DSTATUS disk_initialize(
    BYTE pdrv /* Physical drive nmuber to identify the drive */
)
{
    DSTATUS __attribute__((unused)) stat;
    int value;
    // how many times have we tried?
    int count = 0;
restart:
    count++;
    printf("disk_initialize: attempt %d\n", count);
    if (count > 10)
    {
        printf("disk_initialize: giving up after %d attempts\n", count);
        return STA_NOINIT;
    }
    init_sdcard_io();

    // CRITICAL: Ensure SPI is in 8-bit mode (LCD may have left it in 16-bit mode)
    spi_set_format(sd, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

    // Check if the SPI peripheral has been initialized (enabled).
    if (!(spi_get_hw(sd)->cr1 & SPI_SSPCR1_SSE_BITS))
    {
        printf("disk_initialize: SPI not enabled!\n");
        return RES_NOTRDY;
    }
    
    // IMPORTANT: Send 80+ clock pulses with CS HIGH to initialize card
    disable_sdcard(); // CS high
    sleep_ms(10);     // Let card power stabilize

    // Send at least 74 clock pulses (we send 80) with CS HIGH
    for (int i = 0; i < 10; i++)
    {
        uint8_t dummy = 0xFF;
        spi_write_blocking(sd, &dummy, 1);
    }

    // Now select the card and send CMD0
    enable_sdcard();
    sleep_us(100); // Small delay after selecting

    value = sdcard_cmd(0, 0x00000000, 0x95); // Go to idle state
    printf("disk_initialize: CMD0 response = 0x%02X (expect 0x01)\n", value);

    if (value != 1)
    {
        disable_sdcard();
        goto restart;
    }

    disable_sdcard();

    sdcard_io_high_speed();

    enable_sdcard();
    value = sdcard_cmd(8, 0x000001aa, 0x87); // Check voltage range
    value = sdcard_r3();
    disable_sdcard();

    do
    {
        enable_sdcard();
        value = sdcard_cmd(55, 0x40000000, 0x01); // Start initialization
        value = sdcard_cmd(41, 0x40000000, 0x01); // Start initialization
        disable_sdcard();
    } while (value != 0);

    enable_sdcard();
    value = sdcard_cmd(58, 0x00000000, 0x01); // get OCR [expect 0x00]
    value = sdcard_r3();                      // read OCR value [expect 0xc0ff8000]
    disable_sdcard();

    enable_sdcard();
    value = sdcard_cmd(16, 0x00000200, 0x01); // set block size [expect 0x00]
    disable_sdcard();

    sdcard_status = RES_OK;
    return RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status(
    BYTE pdrv /* Physical drive nmuber to identify the drive */
)
{
    if (sdcard_status != 0)
    {
        sdcard_status = disk_initialize(pdrv);
    }
    // Read the OCR to check if the card is still accessible.
    enable_sdcard();
    if (sdcard_cmd(58, 0x00000000, 0x01) == 0)
    {
        sdcard_r3();
        disable_sdcard();
        return RES_OK;
    }
    disable_sdcard();

    sdcard_status = STA_NOINIT;
    return disk_initialize(pdrv);
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read(
    BYTE pdrv,    /* Physical drive nmuber to identify the drive */
    BYTE *buffer, /* Data buffer to store read data */
    LBA_t sector, /* Start sector in LBA */
    UINT count    /* Number of sectors to read */
)
{
    DRESULT __attribute__((unused)) res;
    int value;
    int status = RES_OK;
    if (disk_status(pdrv) == STA_NOINIT)
        return RES_NOTRDY;
    enable_sdcard();
    for (int c = 0; c < count; c++)
    {
        BYTE *p = &buffer[512 * c];
        value = sdcard_cmd(17, sector + c, 0x01);
        if (value != 0)
        {
            status = RES_ERROR;
            break;
        }
        value = sdcard_readblock(p, 512);
        if (value != 0xfe)
        {
            status = RES_ERROR;
            break;
        }
    }
    disable_sdcard();
    return status;
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write(
    BYTE pdrv,          /* Physical drive nmuber to identify the drive */
    const BYTE *buffer, /* Data to be written */
    LBA_t sector,       /* Start sector in LBA */
    UINT count          /* Number of sectors to write */
)
{
    DRESULT __attribute__((unused)) res;
    BYTE __attribute__((unused)) * p;
    int value;
    int status = RES_OK;
    if (disk_status(pdrv) == STA_NOINIT)
        return RES_NOTRDY;
    enable_sdcard();
    for (int c = 0; c < count; c++)
    {
        const BYTE *p = &buffer[512 * c];
        value = sdcard_cmd(24, sector + c, 0x01);
        if (value != 0)
        {
            printf("disk_write: CMD24 error %d at sector %lu\n", value, (unsigned long)(sector + c));
            status = RES_ERROR;
            break;
        }
        value = sdcard_writeblock(p, 512);
        if (value != 0x05)
        {
            printf("disk_write: writeblock status %d\n", value);
            status = RES_ERROR;
            break;
        }
    }
    disable_sdcard();
    return status;
}

#endif

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl(
    BYTE pdrv, /* Physical drive nmuber (0..) */
    BYTE cmd,  /* Control code */
    void *buff /* Buffer to send/receive control data */
)
{
    DRESULT res = RES_ERROR;
    BYTE __attribute__((unused)) n, csd[16];
    int __attribute__((unused)) result;

    if (disk_status(pdrv) & STA_NOINIT)
        return RES_NOTRDY;

    switch (cmd)
    {
    case CTRL_SYNC:    // make sure there is no pending write process
        return RES_OK; // can't really do much with this now

    case GET_SECTOR_COUNT: // Get the number of sectors on the disk
        if (sdcard_cmd(9, 0x00000000, 0x1) != 0)
            break;
        else
        {
            sdcard_readblock(csd, 16);
            int cs = csd[9] + (((int)csd[8]) << 8) + (((int)csd[7]) << 16) + 1;
            *(int *)buff = cs << 9;
            return RES_OK;
        }

    case GET_BLOCK_SIZE:    // Get the erase block size (in sectors)
        *(int *)buff = 256; // let's go with 128 kb
        return RES_OK;

    default:
        return RES_PARERR;
    }
    return res;
}