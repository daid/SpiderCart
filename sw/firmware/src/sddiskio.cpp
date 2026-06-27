#include <stdio.h>
#include "fatfs/ff.h"
#include "fatfs/diskio.h"
#include "pins.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"


DSTATUS disk_init_status = STA_NOINIT;
int sdcard_type = 0; //SD1, SD2, SDHC

static uint8_t sdcardCommand(uint8_t cmd, uint32_t arg)
{
    uint8_t buffer[6];

    gpio_put(PIN_SD_CS, false);

    auto timeout = make_timeout_time_ms(300);
    // Wait not busy
    while(timeout > get_absolute_time()) {
        spi_read_blocking(spi1, 0xFF, buffer, 1);
        if (buffer[0] == 0xFF) break;
    }
    if (timeout <= get_absolute_time()) {
        gpio_put(PIN_SD_CS, true);
        return 0xFF;
    }

    buffer[0] = cmd | 0x40;
    buffer[1] = arg >> 24;
    buffer[2] = arg >> 16;
    buffer[3] = arg >> 8;
    buffer[4] = arg >> 0;
    uint8_t crc = 0xFF;
    if (cmd == 0x00) crc = 0X95;  // correct crc for CMD0 with arg 0
    if (cmd == 0x08) crc = 0X87;  // correct crc for CMD8 with arg 0X1AA
    buffer[5] = crc;
    spi_write_blocking(spi1, buffer, 6);
    for(int n=0; n<255; n++) {
        spi_read_blocking(spi1, 0xFF, buffer, 1);
        if (!(buffer[0] & 0x80)) break;
    }
    return buffer[0];
}

static uint8_t wait_not_busy(uint32_t timeout_time)
{
    uint8_t status = 0xFF;
    auto timeout = make_timeout_time_ms(timeout_time);

    // Wait not busy
    while(timeout > get_absolute_time()) {
        spi_read_blocking(spi1, 0xFF, &status, 1);
        if (status != 0xFF) break;
    }
    return status;
}

DSTATUS disk_initialize(BYTE pdrv)
{
    (void)pdrv;
    uint8_t buffer[4];
    int status;
 
    spi_init(spi1, 100 * 1000);
 
    gpio_put(PIN_SD_CS, true);
    // must supply min of 74 clock cycles with CS high.
    for (uint8_t i = 0; i < 10; i++) spi_write_blocking(spi1, (const uint8_t*)"\xFF", 1);

    // command to go idle in SPI mode
    auto timeout = make_timeout_time_ms(300);
    while ((status = sdcardCommand(0, 0)) != 0x01) {
        if (timeout < get_absolute_time()) {
            disk_init_status = STA_NODISK;
            gpio_put(PIN_SD_CS, true);
            return disk_init_status;
        }
    }
    gpio_put(PIN_SD_CS, true);
    sleep_ms(1);

    //CMD8 to check for SD version
    if (sdcardCommand(8, 0x1AA) == 0x01) {
        spi_read_blocking(spi1, 0xFF, buffer, 4);
        if (buffer[2] != 0x01) goto fail;
        if (buffer[3] != 0xAA) goto fail;
        sdcard_type = 2;
    } else {
        sdcard_type = 1;
    }
    gpio_put(PIN_SD_CS, true);
    sleep_ms(1);

    // Command to operational
    timeout = make_timeout_time_ms(3000);
    while(true) {
        // Wait not busy
        sdcardCommand(55, 0);
        if (sdcardCommand(41, sdcard_type == 2 ? 0x40000000 : 0) == 0x00)
            break;
        if (timeout < get_absolute_time()) {
            goto fail;
        }
        gpio_put(PIN_SD_CS, true);
        sleep_ms(1);
    }
    gpio_put(PIN_SD_CS, true);
    sleep_ms(1);

    if (sdcard_type == 2) {
        int res;
        if ((res = sdcardCommand(58, 0)) != 0x00) {
            goto fail;
        }
        spi_read_blocking(spi1, 0xFF, buffer, 4);
        gpio_put(PIN_SD_CS, true);
        if ((buffer[0] & 0xC0) == 0xC0)
            sdcard_type = 3; //SDHC
    }
    spi_init(spi1, 10000 * 1000);
    disk_init_status = 0;
    return disk_init_status;

fail:
    gpio_put(PIN_SD_CS, true);
    disk_init_status = STA_NOINIT;
    return disk_init_status;
}

DSTATUS disk_status(BYTE pdrv)
{
    (void)pdrv;
    return disk_init_status;
}

static DRESULT disk_read_single(BYTE* buff, LBA_t sector)
{
    if (sdcardCommand(17, sector) != 0x00) {
        gpio_put(PIN_SD_CS, true);
        return RES_ERROR;
    }

    if (wait_not_busy(300) != 0xFE) {
        gpio_put(PIN_SD_CS, true);
        return RES_ERROR;
    }
    spi_read_blocking(spi1, 0xFF, buff, 512);
    //CRC
    uint16_t crc;
    spi_read_blocking(spi1, 0xFF, (uint8_t*)&crc, 2);

    gpio_put(PIN_SD_CS, true);
    return RES_OK;
}

static DRESULT disk_write_single(const BYTE* buff, LBA_t sector)
{
    if (sdcardCommand(24, sector) != 0x00) {
        gpio_put(PIN_SD_CS, true);
        return RES_ERROR;
    }

    uint8_t cmd = 0xFE; // block start
    spi_write_blocking(spi1, &cmd, 1);
    spi_write_blocking(spi1, buff, 512);
    uint16_t crc = 0xFFFF;
    spi_write_blocking(spi1, (uint8_t*)&crc, 2);
    
    //read status
    uint8_t status;
    spi_read_blocking(spi1, 0xFF, &status, 1);
    if ((status & 0x1F) != 0x05) {
        gpio_put(PIN_SD_CS, true);
        return RES_ERROR;
    }

    if (wait_not_busy(600) == 0xFF) {
        gpio_put(PIN_SD_CS, true);
        return RES_ERROR;
    }

    //Send CMD13
    if (sdcardCommand(13, sector) != 0x00) {
        gpio_put(PIN_SD_CS, true);
        return RES_ERROR;
    }
    spi_read_blocking(spi1, 0xFF, &status, 1);
    if (status != 0)  {
        gpio_put(PIN_SD_CS, true);
        return RES_ERROR;
    }

    gpio_put(PIN_SD_CS, true);
    return RES_OK;
}

DRESULT disk_read(BYTE pdrv, BYTE* buff, LBA_t sector, UINT count)
{
    (void)pdrv;
    if (disk_init_status) return RES_NOTRDY;
    if (sdcard_type != 3) sector <<= 9;
    while(count) {
        if (disk_read_single(buff, sector) != RES_OK)
            return RES_ERROR;
        count -= 1;
        buff += 512;
        sector += 1;
    }
    return RES_OK;
}

DRESULT disk_write(BYTE pdrv, const BYTE* buff, LBA_t sector, UINT count)
{
    (void)pdrv;
    if (disk_init_status) return RES_NOTRDY;
    if (sdcard_type != 3) sector <<= 9;
    while(count) {
        if (disk_write_single(buff, sector) != RES_OK)
            return RES_ERROR;
        count -= 1;
        buff += 512;
        sector += 1;
    }
    return RES_OK;
}

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void* buff)
{
    (void)pdrv;
    switch(cmd) {
    case CTRL_SYNC:	        /* Complete pending write process (needed at FF_FS_READONLY == 0) */
        break;
    case GET_SECTOR_COUNT:  /* Get media size (needed at FF_USE_MKFS == 1) */
        *(DWORD*)buff = 0;
        if (sdcardCommand(9, 0) == 0) {
            wait_not_busy(300);
            uint8_t csd[16];
            spi_read_blocking(spi1, 0xFF, csd, 16);
            auto csd_ver = (csd[0] & 0xC0) >> 6;
            if (csd_ver == 0) { //V1
                uint8_t read_bl_len = csd[5] & 0x0F;
                uint16_t c_size = ((csd[6] & 0x03) << 10) | (csd[7] << 2) | (csd[8] >> 6);
                uint8_t c_size_mult = ((csd[9] & 0x03) << 1) | ((csd[10] & 0x80) >> 7);
                *(DWORD*)buff = (c_size + 1) << (c_size_mult + read_bl_len - 7);
            } else if (csd_ver == 1) { //V2
                uint32_t c_size = ((csd[7] & 0xFC) << 14) | (csd[8] << 8) | csd[9];
                *(DWORD*)buff = (c_size + 1) << 10;
            }
        } 
        gpio_put(PIN_SD_CS, true);
        break;
    case GET_SECTOR_SIZE:   /* Get sector size (needed at FF_MAX_SS != FF_MIN_SS) */
        *(WORD*)buff = 512;
        break;
    case GET_BLOCK_SIZE:    /* Get erase block size (needed at FF_USE_MKFS == 1) */
        *(WORD*)buff = 1;
        break;
    case CTRL_TRIM:         /* Inform device that the data on the block of sectors is no longer used (needed at FF_USE_TRIM == 1) */
        // CMD32 = Start LBA CMD33 = End LBA CMD38 
        //return RES_PARERR;
        break;
    default:
        return RES_PARERR;
    }
    return RES_OK;
}
