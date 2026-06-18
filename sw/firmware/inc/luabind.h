#pragma once

extern "C" {
#include "../src/lua/lua.h"
#include "../src/lua/lauxlib.h"
}

#include <optional>

struct LuaBindError {
    const char* error = nullptr;
};

template<typename T> struct Convert {};
template<> struct Convert<bool> {
    static int toLua(lua_State* L, bool value) { lua_pushboolean(L, value); return 1; }
    static bool fromLua(lua_State* L, int idx) { return lua_toboolean(L, idx); }
};
template<> struct Convert<int> {
    static int toLua(lua_State* L, int value) { lua_pushinteger(L, value); return 1; }
    static int fromLua(lua_State* L, int idx) { return lua_tonumber(L, idx); }
};
template<> struct Convert<float> {
    static int toLua(lua_State* L, float value) { lua_pushnumber(L, value); return 1; }
    static float fromLua(lua_State* L, int idx) { return lua_tonumber(L, idx); }
};
template<typename T> struct Convert<std::optional<T>> {
    static int toLua(lua_State* L, std::optional<T> value) { if (value.has_value()) return Convert<T>::toLua(L, value.value()); lua_pushnil(L); return 1; }
    static std::optional<T> fromLua(lua_State* L, int idx) { if (idx <= lua_gettop(L) && !lua_isnil(L, idx)) return Convert<T>::fromLua(L, idx); return {}; }
};
template<> struct Convert<const char*> {
    static int toLua(lua_State* L, const char* value) { lua_pushstring(L, value); return 1; }
    static const char* fromLua(lua_State* L, int idx) { return lua_tostring(L, idx); }
};
template<> struct Convert<LuaBindError> {
    static int toLua(lua_State* L, LuaBindError error) { if (error.error) return luaL_error(L, "%s", error.error); return 0; }
};

template<typename FUNC, FUNC f> struct lua_bind {

    static void bind(lua_State* L, const char* name)
    {
        if constexpr (std::is_same_v<FUNC, lua_CFunction>) {
            lua_pushcfunction(L, f);
        } else {
            lua_pushcfunction(L, &callHelper);
        }
        lua_setglobal(L, name);
    }

    static int callHelper(lua_State* L)
    {
        return callHelper2(L, f);
    }

    template<typename RET, typename... ARGS> static int callHelper2(lua_State* L, RET(*)(ARGS...))
    {
        return callHelper3(L, f, std::make_index_sequence<sizeof...(ARGS)>());
    }

    template<typename RET, typename... ARGS, size_t... N> static int callHelper3(lua_State* L, RET(*)(ARGS...), std::index_sequence<N...>)
    {
        if constexpr (!std::is_void_v<RET>) {
            auto res = f(Convert<ARGS>::fromLua(L, N + 1)...);
            return Convert<RET>::toLua(L, res);
        } else {
            f(Convert<ARGS>::fromLua(L, N + 1)...);
            return 0;
        }
    }
};

#define LUA_BIND(L, f, name) lua_bind<decltype(&f), f>::bind(L, name);