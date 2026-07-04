cmake_minimum_required(VERSION 3.13)
project(firmwareTest)

add_executable(p8luaTest
    p8lua.cpp

    ../src/p8/p8lua.cpp
    ../src/lua/lapi.c
    ../src/lua/lauxlib.c
    ../src/lua/ldebug.c
    ../src/lua/lfunc.c
    ../src/lua/lvm.c
    ../src/lua/lgc.c
    ../src/lua/ldo.c
    ../src/lua/lobject.c
    ../src/lua/ltable.c
    ../src/lua/lstate.c
    ../src/lua/lstring.c
    ../src/lua/ltm.c
    ../src/lua/lopcodes.c
    ../src/lua/lmem.c
    ../src/lua/lparser.c
    ../src/lua/lzio.c
    ../src/lua/ldump.c
    ../src/lua/lundump.c
    ../src/lua/llex.c
    ../src/lua/lcode.c
    ../src/lua/lctype.c
)
target_include_directories(p8luaTest PUBLIC ../inc)
