#pragma once

#include <stdint.h>

extern "C" {
#include "../src/lua/lua.h"
#include "../src/lua/lauxlib.h"
}

extern uint8_t p8lua_screen[128*128];
static constexpr uint8_t P8LUA_BTN_LEFT  = 0x01;
static constexpr uint8_t P8LUA_BTN_RIGHT = 0x02;
static constexpr uint8_t P8LUA_BTN_UP    = 0x04;
static constexpr uint8_t P8LUA_BTN_DOWN  = 0x08;
static constexpr uint8_t P8LUA_BTN_O     = 0x10;
static constexpr uint8_t P8LUA_BTN_X     = 0x20;
extern uint8_t p8lua_button_mask, p8lua_button_pressed_mask;
extern uint8_t* p8lua_card_data;
extern float p8lua_time;

void setupP8LuaEnv(lua_State* L);