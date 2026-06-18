#pragma once

#include <stdint.h>

extern "C" {
#include "../src/lua/lua.h"
#include "../src/lua/lauxlib.h"
}

extern uint8_t p8lua_screen[128*128];
extern uint8_t* p8lua_card_data;

void setupP8LuaEnv(lua_State* L);