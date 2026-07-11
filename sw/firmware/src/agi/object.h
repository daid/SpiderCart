#pragma once

#include <stdint.h>

namespace AGI {

class Engine;
class Object // viewable ingame object, not to be confused with an inventory item (which are stored in the OBJECT file and named Item)
{
public:
    int16_t x = 0;
    int16_t y = 0;
    uint8_t direction = 0;

    enum class Motion : uint8_t {
        Normal,
        Wander,
        FollowPlayer,
        MoveTo
    } motion = Motion::Normal;
    uint8_t wander_delay = 0;
    uint8_t target_x = 0, target_y = 0;
    uint8_t step_size = 0;
    uint8_t move_finished_flag = 255;
    
    uint8_t view = 0;
    uint8_t loop = 0;
    uint8_t cel = 0;
    enum class CycleMode : uint8_t {
        Normal,
        NormalOnce,
        Reverse,
        ReverseOnce
    } cycle_mode = CycleMode::Normal;
    uint8_t cycle_time = 0;
    uint8_t cycle_counter = 0;
    uint8_t cycle_finished_flag = 255;

    uint8_t priority = 0;

    uint16_t flags = 0;
    static constexpr uint16_t flag_anim = 0x0001;
    static constexpr uint16_t flag_cycling = 0x0002;
    static constexpr uint16_t flag_ignore_objs = 0x0004;
    static constexpr uint16_t flag_ignore_blocks = 0x0008;
    static constexpr uint16_t flag_fix_loop = 0x0010;
    static constexpr uint16_t flag_draw = 0x0020;
    static constexpr uint16_t flag_ignore_horizon = 0x0040;
    static constexpr uint16_t flag_update = 0x0080;
    static constexpr uint16_t flag_fixed_priority = 0x0100;
    static constexpr uint16_t flag_on_land = 0x0200;
    static constexpr uint16_t flag_on_water = 0x0400;
    static constexpr uint16_t flag_dont_update = 0x0800;

    void update(Engine& engine);

    void setView(int view);
    void setLoop(int loop);
    void fixPosition(Engine& engine);

    void animate();

    void move_to(uint8_t target_x, uint8_t target_y, uint8_t step_size, uint8_t finished_flag);
    void stop_motion();

    bool inBox(uint8_t x0, uint8_t x1, uint8_t y0, uint8_t y1);
};

}