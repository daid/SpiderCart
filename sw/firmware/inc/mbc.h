#pragma once

#include <stdint.h>
#include "data_write.pio.h"
#include "data_read.pio.h"

enum class MBC_Type
{
    Spider,
    MBC1,
    MBC2,
    MBC3,
    MBC5,
    Unknown
};

#define MBC_FLAG_RAM     0x0001
#define MBC_FLAG_BATTERY 0x0002
#define MBC_FLAG_TIMER   0x0004
#define MBC_FLAG_RUMBLE  0x0008

void core1_start_mbc(MBC_Type base_type, uint32_t flags, uint32_t rom_bank_mask, uint32_t ram_bank_mask);
