#pragma once

#include <stdint.h>
#include "mbc.h"

extern bool core1_running;

extern MBC_Type mbc_type;
extern uint32_t mbc_flags;
extern uint32_t mbc_rom_bank_mask;
extern uint32_t mbc_ram_bank_mask;

void prepare_mbc();
void start_mbc(bool reset);
void stop_mbc();