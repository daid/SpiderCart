#include "../binjgb/src/emulator.h"
#include "../../sw/firmware/inc/command.h"
#include "../../sw/firmware/inc/p8lua.h"
#include "../../sw/firmware/inc/memchip.h"
#include "../../sw/firmware/inc/coprocessor.h"
#include "../../sw/firmware/inc/mbc_prepare.h"
#include <pico/multicore.h>

#include <cstring>
#include <thread>


FileData* rom_filedata;
bool ram_enabled = false;
uint8_t* ram_ptr = ram_data;
uint8_t current_command = 0;
uint8_t current_command_delay = 0;

extern "C" EmulatorCustomMBC* get_override_mbc(FileData* filedata);

static void mbc_write_rom(Emulator* e, MaskedAddress addr, u8 value)
{
    switch(addr & 0xE000) {
    case 0x0000:
        ram_enabled = (value & 0x0F) == 0x0A;
        break;
    case 0x2000:
        set_rom_bank(e, 1, value & mbc_rom_bank_mask);
        break;
    case 0x4000:
        ram_ptr = ram_data + (value & mbc_ram_bank_mask) * 0x2000;
        break;
    case 0x6000:
        sio_hw->fifo_rd = value | 0x100;
        sio_hw->fifo_valid = true;
        break;
    }
    //printf("MBC Write: %04x: %02x\n", addr, value);
}

extern "C" void emulator_set_PC(Emulator* e, u16 pc);

static u8 mbc_read_ext_ram(Emulator* e, MaskedAddress addr)
{
    if (!ram_enabled) return 0xFF;
    //printf("MBC RAM Read: %04x: %02x\n", addr, ram_ptr[addr]);
    return ram_ptr[addr];
}

static void mbc_write_ext_ram(Emulator* e, MaskedAddress addr, u8 value)
{
    if (!ram_enabled) return;
    ram_ptr[addr] = value;
    //printf("MBC RAM Write: %04x: %02x\n", addr, value);
}

static EmulatorCustomMBC mbc = {
    .read_ext_ram = &mbc_read_ext_ram,
    .write_rom = &mbc_write_rom,
    .write_ext_ram = &mbc_write_ext_ram
};

static void coprocessorThread()
{
    while(true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        processCoProcessor();
    }
}

EmulatorCustomMBC* get_override_mbc(FileData* filedata)
{
    if (filedata->size < 0x200) return nullptr;
    if (filedata->data[0x147] != 0x1A) return nullptr; //SpiderCartMBC should indicate MBC5+RAM
    if (filedata->data[0x150] != 0xDD) return nullptr; //Common first instruction should be invalid
    if (memcmp(&filedata->data[0x151], "SPIDER", 6) != 0) return nullptr; // No spider indicator
    printf("Loading rom with SpiderCartMBC\n");
    filedata->data[0x148] = 0x08;
    //We resize the rom so we can load any size rom in it in the future.
    filedata->size = 8*1024*1024;
    filedata->data = (u8*)realloc(filedata->data, filedata->size);
    rom_filedata = filedata;

    new std::thread(coprocessorThread);

    return &mbc;
}

void write_memchip(uint32_t addr, const uint8_t* data, size_t size)
{
    memcpy(rom_filedata->data + addr, data, size);
}

void read_memchip(uint32_t addr, uint8_t* data, size_t size)
{
    memcpy(data, rom_filedata->data + addr, size);
}
