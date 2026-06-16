cmake_minimum_required(VERSION 3.14)
project(SpiderCartLoader LANGUAGES NONE)

find_package(Python3)

set(GB_HLA_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../GB.HLA/" CACHE FILEPATH "Path to GB.HLA installation")
set(SDCC_DIR "c:/gbdk/bin" CACHE FILEPATH "Path to SDCC installation")

if (NOT EXISTS ${GB_HLA_DIR})
    message(FATAL_ERROR "Missing GB.HLA installation, supply path with -DGB_HLA_DIR= or install it at ${GB_HLA_DIR}")
endif()
if (NOT EXISTS ${SDCC_DIR})
    message(FATAL_ERROR "Missing SDCC installation, supply path with -DSDCC_DIR= or install it at ${SDCC_DIR}")
endif()
add_custom_target(main.rel ALL ${SDCC_DIR}/sdcc ${CMAKE_CURRENT_LIST_DIR}/main.c -msm83 -c -o main.rel)
add_custom_target(Loader.gb ALL Python3::Interpreter ${GB_HLA_DIR}/main.py ${CMAKE_CURRENT_LIST_DIR}/main.asm --output Loader.gb --symbols Loader.sym --include-path ${CMAKE_CURRENT_BINARY_DIR} DEPENDS main.rel)
