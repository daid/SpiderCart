#pragma once

#include <cstdint>

extern uint8_t ram_data[16 * 0x2000];

int co_error(int error_nr, const char* fmt, ...);
void processCoProcessor();