cmake_minimum_required(VERSION 3.14)
project(SpiderCartP8 LANGUAGES NONE)

find_package(Python3)

set(GB_HLA_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../GB.HLA/" CACHE FILEPATH "Path to GB.HLA installation")

if (NOT EXISTS ${GB_HLA_DIR})
    message(FATAL_ERROR "Missing GB.HLA installation, supply path with -DGB_HLA_DIR= or install it at ${GB_HLA_DIR}")
endif()
file(GLOB_RECURSE ASM_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.asm ${CMAKE_CURRENT_SOURCE_DIR}/src/*.png)
add_custom_command(OUTPUT p8.gb COMMAND Python3::Interpreter ${GB_HLA_DIR}/main.py ${CMAKE_CURRENT_LIST_DIR}/src/main.asm --output p8.gb --symbols p8.sym --include-path ${CMAKE_CURRENT_BINARY_DIR} DEPENDS ${ASM_SOURCES})
add_custom_target(p8 ALL DEPENDS p8.gb)