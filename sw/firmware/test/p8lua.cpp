#include "p8lua.h"
#include <cstring>
#include <string>

#define FAIL "\x1B[1;31mFAIL\x1B[22;39m"

int total_test_count = 0;
int total_fail_count = 0;
void runLuaTest(const char* name, const char* code, const char* expected_error = nullptr);

int main(int argc, char** argv)
{
    runLuaTest("assert true", "assert(true, [[FAIL]])");
    runLuaTest("assert false", "assert(false, [[FAIL]])", ":1: FAIL");
    runLuaTest("add", "a = {}\nadd(a, 2)\nassert(#a == 1, [[len]])\nassert(a[1] == 2, [[value]])");
    runLuaTest("add2", "a = {}\nadd(a, 2)\nadd(a, 4)\nassert(#a == 2, [[len]])\nassert(a[2] == 4, [[value]])");
    runLuaTest("del", "a = {1,2,3}\ndel(a, 2)\nassert(#a == 2, [[len]])\nassert(a[1] == 1, [[value]])\nassert(a[2] == 3, [[value]])");
    runLuaTest("del2", "a = {1,2,3,2}\ndel(a, 2)\nassert(#a == 3, [[len]])\nassert(a[3] == 2, [[value]])");
    runLuaTest("del last", "a = {1,2,3}\ndel(a, 3)\nassert(#a == 2, [[len]])\nassert(a[1] == 1, [[value]])\nassert(a[2] == 2, [[value]])");
    runLuaTest("del nil", "a = {1,2,3}\ndel(a, nil)\nassert(#a == 3, [[len]] .. #a)\nassert(a[1] == 1, [[value]])\nassert(a[2] == 2, [[value]])\nassert(a[3] == 3, [[value]])");
    runLuaTest("foreach del", "a = {1,2,3}\nforeach(a, function(n) print(tostr(n)) del(a, n) end)\nassert(#a == 0, [[len]] .. #a)");
    runLuaTest("+=", "a = 1\na += 1\nassert(a == 2)");
    runLuaTest("-=", "a = 1\na -= 1\nassert(a == 0)");
    runLuaTest("short if", "a = 0\nif (true) a = 1\nassert(a == 1)");
    runLuaTest("short if else1", "a = 0\nif (true) a = 1 else a = 2\nassert(a == 1)");
    runLuaTest("short if else2", "a = 0\nif (false) a = 1 else a = 2\nassert(a == 2)");
    runLuaTest("rnd", "assert(rnd(1) != rnd(1))");
    runLuaTest("sin", "assert(sin(0.25) == -1, sin(0.25))");
    runLuaTest("sin", "assert(sin(0.75) == 1, sin(0.75))");
    runLuaTest("foreach", "a = 0\nforeach({1, 2, 3}, function(n) a += n end)\nassert(a == 6)");
    if (total_fail_count)
        printf("\x1B[1;31mFAILED: %d\x1B[22;39m\n", total_fail_count);
    return total_fail_count ? 1 : 0;
}

std::string runLua(const char* code)
{
    total_test_count += 1;
    auto L = luaL_newstate();
    setupP8LuaEnv(L);

    auto res = luaL_loadbuffer(L, code, strlen(code), "=");
    if (res) {
        std::string res = lua_tostring(L, -1);
        lua_close(L);
        return res;
    }
    res = lua_pcall(L, 0, 0, 0);
    if (res) {
        std::string res = lua_tostring(L, -1);
        lua_close(L);
        return res;
    }
    lua_close(L);
    return "";
}

void runLuaTest(const char* name, const char* code, const char* expected_error)
{
    total_test_count += 1;

    auto res = runLua(code);
    if (expected_error) {
        if (strcmp(expected_error, res.c_str()) == 0) {
            res = "";
        } else {
            res = "Expected error: " + std::string(expected_error) + " got: " + res;
        }
    }
    if (res.empty()) {
        printf("PASS: %s\n", name);
    } else {
        printf(FAIL ": %s: %s\n", name, res.c_str());
        total_fail_count += 1;
    }
}
