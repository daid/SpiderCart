#pragma once

#include <stdint.h>

namespace AGI {

class Object // viewable ingame object, not to be confused with an inventory item (which are stored in the OBJECT file and named Item)
{
public:
    int16_t x = 0;
    int16_t y = 0;
    uint8_t direction = 0;
    
    uint8_t view = 0;
    uint8_t loop = 0;
    uint8_t cel = 0;
    uint8_t cycle_time = 0;
    uint8_t cycle_counter = 0;

    uint8_t priority = 0;

    uint16_t flags = 0;
    static constexpr uint16_t flag_anim = 0x0001;
    static constexpr uint16_t flag_cycling = 0x0002;
    static constexpr uint16_t flag_observes = 0x0004;
    static constexpr uint16_t flag_ignore_blocks = 0x0008;
    static constexpr uint16_t flag_fix_loop = 0x0010;
    static constexpr uint16_t flag_draw = 0x0020;

    void setLoop(int loop);

    void move_to(uint8_t target_x, uint8_t target_y, uint8_t step_size, uint8_t finished_flag);
    void stop_motion();
};

}