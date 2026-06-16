#pragma once

extern "C" {
#include "../src/lua/lua.h"
#include "../src/lua/lauxlib.h"
}

void setupP8LuaEnv(lua_State* L);