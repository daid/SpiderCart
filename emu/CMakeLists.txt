cmake_minimum_required(VERSION 3.11)
project(SpiderCartEmu)
add_subdirectory(binjgb)
add_subdirectory(../sw/loader loader)

set(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mbc.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../sw/firmware/src/lua/lapi.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../sw/firmware/src/lua/lauxlib.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../sw/firmware/src/lua/ldebug.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../sw/firmware/src/lua/lfunc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../sw/firmware/src/lua/lvm.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../sw/firmware/src/lua/lgc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../sw/firmware/src/lua/ldo.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../sw/firmware/src/lua/lobject.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../sw/firmware/src/lua/ltable.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../sw/firmware/src/lua/lstate.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../sw/firmware/src/lua/lstring.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../sw/firmware/src/lua/ltm.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../sw/firmware/src/lua/lopcodes.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../sw/firmware/src/lua/lmem.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../sw/firmware/src/lua/lparser.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../sw/firmware/src/lua/lzio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../sw/firmware/src/lua/ldump.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../sw/firmware/src/lua/lundump.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../sw/firmware/src/lua/llex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../sw/firmware/src/lua/lcode.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../sw/firmware/src/lua/lctype.c
)

target_sources(binjgb PUBLIC ${SOURCES})
target_sources(binjgb-debugger PUBLIC ${SOURCES})
