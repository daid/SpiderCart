#include "mbc_prepare.h"
#include "memchip.h"
#include "usb_msc.h"
#include "accelerometer.h"
#include <pico/multicore.h>
#include <chrono>
#include <string>
#include <cstring>

static sio_hw_t sio_hw_impl;
sio_hw_t* sio_hw = &sio_hw_impl;

bool multicore_fifo_rvalid() {
    if (sio_hw->fifo_valid) {
        sio_hw->fifo_valid = false;
        return true;
    }
    return false;
}

absolute_time_t get_absolute_time(void)
{
    auto now = std::chrono::steady_clock::now();
    return now.time_since_epoch().count() / 1000;
}

void accelerometer_read(int16_t data[3])
{
    data[0] = 0;
    data[1] = 0;
    data[2] = 0x4000;
}

bool core1_running = true;

MBC_Type mbc_type = MBC_Type::Spider;
uint32_t mbc_flags = MBC_FLAG_RAM;
uint32_t mbc_rom_bank_mask = 0xFF;
uint32_t mbc_ram_bank_mask = 0x0F;

void prepare_mbc()
{
    printf("prepare_mbc\n");
    uint8_t header_info[0x100];
    read_memchip(0x100, header_info, 0x100);
    printf("%s\n", header_info);
}

void start_fake_reset(void);

void start_mbc(bool reset)
{
    core1_running = true;
    printf("start_mbc\n");
    if (reset) {
        start_fake_reset();
    }
}

void stop_mbc()
{
    core1_running = false;
    printf("stop_mbc\n");
}

static bool msc_enabled = false;

bool is_usb_msc_enabled()
{
    return msc_enabled;
}

void set_usb_msc_enable(bool enabled)
{
    msc_enabled = enabled;
}

