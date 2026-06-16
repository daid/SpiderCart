#include "../binjgb/src/emulator.h"
#include "../../sw/firmware/inc/command.h"

#include <dirent.h>


FileData rom_filedata;
bool ram_enabled = false;
uint8_t ram_data[16 * 0x2000];
uint8_t* ram_ptr = ram_data;
uint8_t current_command = 0;
uint8_t current_command_delay = 0;

extern "C" EmulatorCustomMBC* get_override_mbc(FileData filedata);

bool validExt(char* sep)
{
    if (!sep) return false;
    if (strcmpi(sep, ".gb") == 0) return true;
    if (strcmpi(sep, ".gbc") == 0) return true;
    return false;
}

static void executeCurrentCommand()
{
    switch (current_command)
    {
    case COMMAND_LIST_DIR:
        {
            uint8_t* ptr = ram_data;
            auto dir = opendir(".");
            while(auto entry = readdir(dir)) {
                //*ptr++ = (entry->d_type & DT_DIR) ? 2 : 1;
                auto sep = strrchr(entry->d_name, '.');
                if (validExt(sep)) {
                    *ptr++ = 1;
                    strncpy((char*)ptr, entry->d_name, 31);
                    printf("%s\n", entry->d_name);
                    ptr += 31;
                }
            }
            *ptr = 0;
            closedir(dir);
        }
        //Mark command done.
        ram_data[15 * 0x2000 + 0x1FFF] = 0;
        break;
    case COMMAND_LOAD_AND_RESET:
        printf("Want to load&reset: %s\n", &ram_data[15 * 0x2000]);
        //Mark command done.
        ram_data[15 * 0x2000 + 0x1FFF] = 0;
        break;
    default:
        break;
    }
}

static void mbc_write_rom(Emulator* e, MaskedAddress addr, u8 value)
{
    switch(addr & 0xE000) {
    case 0x0000:
        ram_enabled = (value & 0x0F) == 0x0A;
        break;
    case 0x2000:
        set_rom_bank(e, 1, value);
        break;
    case 0x4000:
        ram_ptr = ram_data + (value & 0x0F) * 0x2000;
        break;
    case 0x6000:
        current_command = value;
        current_command_delay = 16;
        break;
    }
    printf("MBC Write: %04x: %02x\n", addr, value);
}

static u8 mbc_read_ext_ram(Emulator* e, MaskedAddress addr)
{
    if (!ram_enabled) return 0xFF;
    if (current_command) {
        if (current_command_delay) {
            current_command_delay -= 1;
        }else{
            executeCurrentCommand();
            current_command = 0;
        }
    }
    printf("MBC RAM Read: %04x: %02x\n", addr, ram_ptr[addr]);
    return ram_ptr[addr];
}

static void mbc_write_ext_ram(Emulator* e, MaskedAddress addr, u8 value)
{
    if (!ram_enabled) return;
    ram_ptr[addr] = value;
    printf("MBC RAM Write: %04x: %02x\n", addr, value);
}

static EmulatorCustomMBC mbc = {
    .read_ext_ram = &mbc_read_ext_ram,
    .write_rom = &mbc_write_rom,
    .write_ext_ram = &mbc_write_ext_ram
};

EmulatorCustomMBC* get_override_mbc(FileData filedata)
{
    if (filedata.size < 0x200) return nullptr;
    if (filedata.data[0x147] != 0x1A) return nullptr; //SpiderCartMBC should indicate MBC5+RAM
    if (filedata.data[0x150] != 0xDD) return nullptr; //Common first instruction should be invalid
    if (memcmp(&filedata.data[0x151], "SPIDER", 6) != 0) return nullptr; // No spider indicator
    printf("Loading rom with SpiderCartMBC\n");
    rom_filedata = filedata;
    return &mbc;
}
