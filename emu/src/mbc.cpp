#include "../binjgb/src/emulator.h"


extern "C" EmulatorCustomMBC* get_override_mbc();

static void mbc_write_rom(Emulator*, MaskedAddress addr, u8 value)
{
    printf("MBC Write: %04x: %02x\n", addr, value);
}

static u8 mbc_read_ext_ram(Emulator*, MaskedAddress addr)
{
    printf("MBC RAM Read: %04x\n", addr);
    return 0;
}

static void mbc_write_ext_ram(Emulator*, MaskedAddress addr, u8 value)
{
    printf("MBC RAM Write: %04x: %02x\n", addr, value);
}

static EmulatorCustomMBC mbc = {
    .read_ext_ram = &mbc_read_ext_ram,
    .write_rom = &mbc_write_rom,
    .write_ext_ram = &mbc_write_ext_ram
};

EmulatorCustomMBC* get_override_mbc()
{
    return &mbc;
}
