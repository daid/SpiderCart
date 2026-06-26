#pragma once

#include <stdint.h>

typedef uint64_t absolute_time_t;

absolute_time_t get_absolute_time(void);

static int64_t absolute_time_diff_us (absolute_time_t from, absolute_time_t to)
{
    return to - from;
}

static inline absolute_time_t make_timeout_time_ms(uint32_t ms) {
    return get_absolute_time() + ms * 1000;
}