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
    runLuaTest("add3", "a = {1,2,3}\nadd(a, 4, 2)\nassert(#a == 4, [[len]])\nassert(a[2] == 4, [[value]])\nassert(a[3] == 2, [[value]])\nassert(a[4] == 3, [[value]])");
    runLuaTest("del", "a = {1,2,3}\ndel(a, 2)\nassert(#a == 2, [[len]])\nassert(a[1] == 1, [[value]])\nassert(a[2] == 3, [[value]])");
    runLuaTest("del2", "a = {1,2,3,2}\ndel(a, 2)\nassert(#a == 3, [[len]])\nassert(a[3] == 2, [[value]])");
    runLuaTest("del last", "a = {1,2,3}\ndel(a, 3)\nassert(#a == 2, [[len]])\nassert(a[1] == 1, [[value]])\nassert(a[2] == 2, [[value]])");
    runLuaTest("del nil", "a = {1,2,3}\ndel(a, nil)\nassert(#a == 3, [[len]] .. #a)\nassert(a[1] == 1, [[value]])\nassert(a[2] == 2, [[value]])\nassert(a[3] == 3, [[value]])");
    runLuaTest("+=", "a = 1\na += 1\nassert(a == 2)");
    runLuaTest("-=", "a = 1\na -= 1\nassert(a == 0)");
    runLuaTest("*=", "a = 2\na *= 3\nassert(a == 6)");
    runLuaTest("/=", "a = 6\na /= 2\nassert(a == 3)");
    runLuaTest("%=", "a = 7\na %= 3\nassert(a == 1)");
    runLuaTest("..=", "a = 'a'\na ..= 'b'\nassert(a == 'ab')");
    runLuaTest("short if", "a = 0\nif (true) a = 1\nassert(a == 1)");
    runLuaTest("short if else1", "a = 0\nif (true) a = 1 else a = 2\nassert(a == 1)");
    runLuaTest("short if else2", "a = 0\nif (false) a = 1 else a = 2\nassert(a == 2)");
    runLuaTest("rnd", "assert(rnd(1) != rnd(1))");
    runLuaTest("sin", "assert(sin(0.25) == -1, sin(0.25))");
    runLuaTest("sin", "assert(sin(0.75) == 1, sin(0.75))");
    runLuaTest("foreach", "a = 0\nforeach({1, 2, 3}, function(n) a += n end)\nassert(a == 6)");
    runLuaTest("foreach del", "a = {1,2,3}\nforeach(a, function(n) del(a, n) end)\nassert(#a == 0, [[len]] .. #a)");
    runLuaTest("all", "n = 0 for a in all({1, 2, 3}) do n += 1 end assert(n == 3)");
    runLuaTest("all del", "a = {1,2,3}\nfor n in all(a) do del(a, n) end\nassert(#a == 0, [[len]] .. #a)");
    runLuaTest("number parse oddness", "a = 1b=2\nassert(b == 2)");
    runLuaTest("number parse oddness2", "a = 0e=2\nassert(e == 2)");
    runLuaTest("if .. do .. end", "a = 0\nif a == 0 do a = 2 end\nassert(a==2)");
    runLuaTest("while (...) ...", "a = 0\nwhile (a < 2) a += 1");
    runLuaTest("int div", "a = 10 \\ 3\nassert(a == 3)");
    runLuaTest("split", "a = split(\"a,b,1,2\", \",\")\nassert(#a == 4)\nassert(a[1] == \"a\", 1)\nassert(a[2] == \"b\", 2)\nassert(a[3] == 1, 3)\nassert(a[4] == 2, 4)");
    runLuaTest("?", "?\"1\"\n?1\n?1,2,3");
    runLuaTest("special chars", "assert(\x81 == 23130.5)");
    runLuaTest("special chars 2", "\x81\x82 = 3\nassert(\x81\x82 == 3)");
    runLuaTest("escape sequence", "a = \"\\0\"\na = \"\\*\"\na = \"\\#\\-\\|\\+\\^\\a\\b\\t\\n\\v\\f\\r\\014\\015\"");
    runLuaTest("binary number int", "a = 0b100\nassert(a == 4)");
    runLuaTest("binary number flt", "a = 0b100.1\nb = 0b1.001\nassert(a == 4.5)\nassert(b == 1.125)");
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
