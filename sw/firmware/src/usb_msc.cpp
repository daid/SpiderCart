#include "usb_msc.h"
#include <tusb.h>
extern "C" {
#include <class/msc/msc_device.h>
}
#include "fatfs/ff.h"
#include "fatfs/diskio.h"

static bool msc_enabled = false;

bool is_usb_msc_enabled()
{
    return msc_enabled;
}

void set_usb_msc_enable(bool enabled)
{
    msc_enabled = enabled;
}


// Copy disk's data to buffer (up to bufsize) and return number of copied bytes.
int32_t tud_msc_read10_cb(uint8_t lun, uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize)
{
    (void)lun;
    (void)offset;
    (void)bufsize;
    if (!msc_enabled) return 0;

    if (disk_read(0, (BYTE*)buffer, lba, 1) == RES_OK)
        return 512;
    return 0;
}

// Process data in buffer to disk's storage and return number of written bytes
int32_t tud_msc_write10_cb(uint8_t lun, uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize)
{
    (void)lun;
    (void)offset;
    (void)bufsize;
    if (!msc_enabled) return 0;

    if (disk_write(0, (BYTE*)buffer, lba, 1) == RES_OK)
        return 512;
    return 0;
}

int32_t tud_msc_scsi_cb(uint8_t lun, uint8_t const scsi_cmd[16], void* buffer, uint16_t bufsize)
{
    (void)lun;
    (void)scsi_cmd;
    (void)buffer;
    (void)bufsize;
    return -1;
}

void tud_msc_capacity_cb(uint8_t lun, uint32_t* block_count, uint16_t* block_size)
{
    (void)lun;
    *block_count = 0;
    *block_size  = 512;
    if (!msc_enabled) return;

    if (disk_ioctl(0, GET_SECTOR_COUNT, &block_count) != 0)
        *block_count = 0;
}

bool tud_msc_test_unit_ready_cb(uint8_t lun)
{
    (void)lun;
    if (!msc_enabled) return false;
    return disk_status(0) == 0;
}

// Fill vendor id, product id, revision with string up to 8, 16, 4 characters respectively
void tud_msc_inquiry_cb(uint8_t lun, uint8_t vendor_id[8], uint8_t product_id[16], uint8_t product_rev[4])
{
    (void)lun;
    const char vid[] = "Daid";
    const char pid[] = "SpiderCart";
    const char rev[] = "0.1";

    memcpy(vendor_id,   vid, strlen(vid));
    memcpy(product_id,  pid, strlen(pid));
    memcpy(product_rev, rev, strlen(rev));
}
