#pragma once

#include <stdint.h>
#include <stdlib.h>


void write_memchip(uint32_t addr, const uint8_t* data, size_t size);
void read_memchip(uint32_t addr, uint8_t* data, size_t size);
