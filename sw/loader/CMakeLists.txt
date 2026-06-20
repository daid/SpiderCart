cmake_minimum_required(VERSION 3.14)
project(SpiderCartLoader LANGUAGES NONE)

find_package(Python3)

set(GB_HLA_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../GB.HLA/" CACHE FILEPATH "Path to GB.HLA installation")
set(SDCC_DIR "c:/gbdk/bin" CACHE FILEPATH "Path to SDCC installation")
find_program(SDCC sdcc HINTS ${SDCC_DIR} REQUIRED)

if (NOT EXISTS ${GB_HLA_DIR})
    message(FATAL_ERROR "Missing GB.HLA installation, supply path with -DGB_HLA_DIR= or install it at ${GB_HLA_DIR}")
endif()
file(GLOB_RECURSE ASM_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.asm ${CMAKE_CURRENT_SOURCE_DIR}/src/*.png)
file(GLOB_RECURSE C_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h)
add_custom_command(OUTPUT main.rel COMMAND ${SDCC} ${CMAKE_CURRENT_LIST_DIR}/src/main.c -msm83 -c -o main.rel DEPENDS ${C_SOURCES})
add_custom_command(OUTPUT Loader.gb COMMAND Python3::Interpreter ${GB_HLA_DIR}/main.py ${CMAKE_CURRENT_LIST_DIR}/src/main.asm --output Loader.gb --symbols Loader.sym --include-path ${CMAKE_CURRENT_BINARY_DIR} DEPENDS ${ASM_SOURCES} main.rel)
add_custom_target(Loader ALL DEPENDS Loader.gb)