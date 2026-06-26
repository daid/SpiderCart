#pragma once

#include <pico/time.h>

bool multicore_fifo_rvalid();

struct sio_hw_t {
    uint32_t fifo_rd;
    bool fifo_valid;
};
extern sio_hw_t* sio_hw;