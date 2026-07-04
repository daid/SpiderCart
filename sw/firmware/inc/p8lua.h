#pragma once

#include <stdint.h>

extern "C" {
#include "../src/lua/lua.h"
#include "../src/lua/lauxlib.h"
}

class P8State
{
public:
    static constexpr uint8_t BTN_LEFT  = 0x01;
    static constexpr uint8_t BTN_RIGHT = 0x02;
    static constexpr uint8_t BTN_UP    = 0x04;
    static constexpr uint8_t BTN_DOWN  = 0x08;
    static constexpr uint8_t BTN_O     = 0x10;
    static constexpr uint8_t BTN_X     = 0x20;

    uint8_t screen[128*128];
    uint8_t button_mask, button_pressed_mask;
    uint8_t repeat_delay = 0;

    uint8_t* card_data;
    float time;

    struct P8SfxChannel {
        bool active = false;
        int sfx_nr = 0;
        int sample_idx = 0;
        int speed_counter = 0;
    };
    P8SfxChannel sfx[4];
};
extern P8State p8_state;

void setupP8LuaEnv(lua_State* L);