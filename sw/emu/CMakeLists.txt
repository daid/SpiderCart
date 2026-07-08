cmake_minimum_required(VERSION 3.11)
project(SpiderCartEmu)
set(CMAKE_CXX_STANDARD 17)
add_subdirectory(binjgb)
find_package(Python3)
add_subdirectory(../loader loader)
add_custom_command(OUTPUT Loader.gb.inc COMMAND Python3::Interpreter ${CMAKE_CURRENT_SOURCE_DIR}/../firmware/rom2inc.py ${CMAKE_CURRENT_BINARY_DIR}/loader/Loader.gb Loader.gb.inc DEPENDS loader loader/Loader.gb ${CMAKE_CURRENT_SOURCE_DIR}/../firmware/rom2inc.py)
add_custom_target(loader.dep DEPENDS Loader.gb.inc)
add_subdirectory(../p8 p8)
add_custom_command(OUTPUT p8.gb.inc COMMAND Python3::Interpreter ${CMAKE_CURRENT_SOURCE_DIR}/../firmware/rom2inc.py ${CMAKE_CURRENT_BINARY_DIR}/p8/p8.gb p8.gb.inc DEPENDS p8 p8/p8.gb ${CMAKE_CURRENT_SOURCE_DIR}/../firmware/rom2inc.py)
add_custom_target(p8.dep DEPENDS p8.gb.inc)
set_property(TARGET binjgb-debugger PROPERTY CXX_STANDARD 17)

set(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mbc.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/simu.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/../firmware/src/coprocessor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../firmware/src/p8/p8.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../firmware/src/p8/p8lua.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../firmware/src/romload.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../firmware/src/filesystem.cpp
    
    ${CMAKE_CURRENT_SOURCE_DIR}/../firmware/src/lua/lapi.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../firmware/src/lua/lauxlib.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../firmware/src/lua/ldebug.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../firmware/src/lua/lfunc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../firmware/src/lua/lvm.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../firmware/src/lua/lgc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../firmware/src/lua/ldo.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../firmware/src/lua/lobject.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../firmware/src/lua/ltable.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../firmware/src/lua/lstate.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../firmware/src/lua/lstring.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../firmware/src/lua/ltm.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../firmware/src/lua/lopcodes.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../firmware/src/lua/lmem.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../firmware/src/lua/lparser.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../firmware/src/lua/lzio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../firmware/src/lua/ldump.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../firmware/src/lua/lundump.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../firmware/src/lua/llex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../firmware/src/lua/lcode.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../firmware/src/lua/lctype.c
)

target_sources(binjgb PUBLIC ${SOURCES})
target_include_directories(binjgb PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc ${CMAKE_CURRENT_SOURCE_DIR}/../firmware/inc ${CMAKE_CURRENT_BINARY_DIR})
add_dependencies(binjgb p8.dep loader.dep)
target_sources(binjgb-debugger PUBLIC ${SOURCES})
target_include_directories(binjgb-debugger PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc ${CMAKE_CURRENT_SOURCE_DIR}/../firmware/inc ${CMAKE_CURRENT_BINARY_DIR})
add_dependencies(binjgb-debugger p8.dep loader.dep)
