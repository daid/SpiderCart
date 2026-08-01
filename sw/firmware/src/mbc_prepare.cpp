#include "mbc_prepare.h"
#include "mbc.h"
#include "pins.h"
#include "memchip.h"

#include "data_write.pio.h"
#include "data_read.pio.h"

#include <string.h>
#include <hardware/gpio.h>
#include <pico/multicore.h>

bool core1_running = false;

MBC_Type mbc_type = MBC_Type::Spider;
uint32_t mbc_flags = 0;
uint32_t mbc_rom_bank_mask = 0xFF;
uint32_t mbc_ram_bank_mask = 0x0F;
static void core1_mbc_handler()
{
    core1_start_mbc(mbc_type, mbc_flags, mbc_rom_bank_mask, mbc_ram_bank_mask);
}

void start_mbc(bool reset) {
    if (!core1_running) {
        if (reset)
            gpio_put(PIN_GB_RST, false);
        multicore_launch_core1(core1_mbc_handler);
        sleep_ms(5);
        if (reset)
            gpio_put(PIN_GB_RST, true);
        core1_running = true;
    }
}

void stop_mbc() {
    if (core1_running) {
        multicore_reset_core1();
        gpio_put_all(PIN_MASK_MEM_OE | PIN_MASK_MEM_WE);
        data_write_set_in(pio0, 0);
        core1_running = false;
    }
}

void prepare_mbc()
{
    if (core1_running) return;
    uint8_t header_info[0x100];
    read_memchip(0x100, header_info, 0x100);
    switch(header_info[0x48]) {
    case 0x00: mbc_rom_bank_mask = 0x000; break; //32 KiB	2 (no banking)
    case 0x01: mbc_rom_bank_mask = 0x003; break; //64 KiB	4
    case 0x02: mbc_rom_bank_mask = 0x007; break; //128 KiB	8
    case 0x03: mbc_rom_bank_mask = 0x00F; break; //256 KiB	16
    case 0x04: mbc_rom_bank_mask = 0x01F; break; //512 KiB	32
    case 0x05: mbc_rom_bank_mask = 0x03F; break; //1 MiB	64
    case 0x06: mbc_rom_bank_mask = 0x07F; break; //2 MiB	128
    case 0x07: mbc_rom_bank_mask = 0x0FF; break; //4 MiB	256
    case 0x08: mbc_rom_bank_mask = 0x1FF; break; //8 MiB	512
    default: mbc_rom_bank_mask = 0x1FF; break;
    }
    switch(header_info[0x49]) {
    case 0x00: mbc_ram_bank_mask = 0x00; break;	// 0	No RAM
    case 0x01: mbc_ram_bank_mask = 0x00; break;	// –	Unused 14
    case 0x02: mbc_ram_bank_mask = 0x00; break;	// 8 KiB	1 bank
    case 0x03: mbc_ram_bank_mask = 0x03; break;	// 32 KiB	4 banks of 8 KiB each
    case 0x04: mbc_ram_bank_mask = 0x0F; break;	// 128 KiB	16 banks of 8 KiB each
    case 0x05: mbc_ram_bank_mask = 0x07; break;	// 64 KiB	8 banks of 8 KiB each
    default: mbc_ram_bank_mask = 0x0F; break;
    }
    mbc_flags = 0;
    switch(header_info[0x47]) {
    case 0x00: mbc_type = MBC_Type::Unknown; mbc_rom_bank_mask = 0x01; break; //ROM ONLY
    case 0x01: mbc_type = MBC_Type::MBC1; break; //MBC1
    case 0x02: mbc_type = MBC_Type::MBC1; mbc_flags = MBC_FLAG_RAM; break; //MBC1+RAM
    case 0x03: mbc_type = MBC_Type::MBC1; mbc_flags = MBC_FLAG_BATTERY; break; //MBC1+RAM+BATTERY
    case 0x05: mbc_type = MBC_Type::MBC2; break; //MBC2
    case 0x06: mbc_type = MBC_Type::MBC2; mbc_flags = MBC_FLAG_BATTERY; break; //MBC2+BATTERY
    case 0x08: mbc_type = MBC_Type::Unknown; mbc_flags = MBC_FLAG_RAM; break; //ROM+RAM 11
    case 0x09: mbc_type = MBC_Type::Unknown; mbc_flags = MBC_FLAG_BATTERY; break; //ROM+RAM+BATTERY 11
    case 0x0B: mbc_type = MBC_Type::Unknown; break; //MMM01
    case 0x0C: mbc_type = MBC_Type::Unknown; mbc_flags = MBC_FLAG_RAM; break; //MMM01+RAM
    case 0x0D: mbc_type = MBC_Type::Unknown; mbc_flags = MBC_FLAG_RAM | MBC_FLAG_BATTERY; break; //MMM01+RAM+BATTERY
    case 0x0F: mbc_type = MBC_Type::MBC3; mbc_flags = MBC_FLAG_TIMER | MBC_FLAG_BATTERY; break; //MBC3+TIMER+BATTERY
    case 0x10: mbc_type = MBC_Type::MBC3; mbc_flags = MBC_FLAG_TIMER | MBC_FLAG_RAM | MBC_FLAG_BATTERY; break; //MBC3+TIMER+RAM+BATTERY 12
    case 0x11: mbc_type = MBC_Type::MBC3; break; //MBC3
    case 0x12: mbc_type = MBC_Type::MBC3; mbc_flags = MBC_FLAG_RAM; break; //MBC3+RAM 12
    case 0x13: mbc_type = MBC_Type::MBC3; mbc_flags = MBC_FLAG_RAM | MBC_FLAG_BATTERY; break; //MBC3+RAM+BATTERY 12
    case 0x19: mbc_type = MBC_Type::MBC5; break; //MBC5
    case 0x1A: mbc_type = MBC_Type::MBC5; mbc_flags = MBC_FLAG_RAM; break; //MBC5+RAM
    case 0x1B: mbc_type = MBC_Type::MBC5; mbc_flags = MBC_FLAG_RAM | MBC_FLAG_BATTERY; break; //MBC5+RAM+BATTERY
    case 0x1C: mbc_type = MBC_Type::MBC5; mbc_flags = MBC_FLAG_RUMBLE; break; //MBC5+RUMBLE
    case 0x1D: mbc_type = MBC_Type::MBC5; mbc_flags = MBC_FLAG_RUMBLE | MBC_FLAG_RAM; break; //MBC5+RUMBLE+RAM
    case 0x1E: mbc_type = MBC_Type::MBC5; mbc_flags = MBC_FLAG_RUMBLE | MBC_FLAG_RAM | MBC_FLAG_BATTERY; break; //MBC5+RUMBLE+RAM+BATTERY
    case 0x20: mbc_type = MBC_Type::Unknown; break; //MBC6
    case 0x22: mbc_type = MBC_Type::MBC7; mbc_flags = MBC_FLAG_RAM; mbc_ram_bank_mask = 0x00; break; //MBC7
    case 0xFC: mbc_type = MBC_Type::Unknown; break; //POCKET CAMERA
    case 0xFD: mbc_type = MBC_Type::Unknown; break; //BANDAI TAMA5
    case 0xFE: mbc_type = MBC_Type::Unknown; break; //HuC3
    case 0xFF: mbc_type = MBC_Type::Unknown; break; //HuC1+RAM+BATTERY
    default: mbc_type = MBC_Type::Unknown; break;
    }

    if (header_info[0x47] == 0x1A && header_info[0x50] == 0xDD && memcmp((char*)&header_info[0x51], "SPIDER", 6) == 0) {
        //Special spider cart override.
        mbc_type = MBC_Type::Spider;
    }
}
