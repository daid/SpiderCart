#pragma once

#include <stdint.h>

extern "C" {
#include "../src/lua/lua.h"
#include "../src/lua/lauxlib.h"
}

class P8LuaState
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

    uint8_t* card_data;
    float time;
};
extern P8LuaState p8lua_state;

void setupP8LuaEnv(lua_State* L);