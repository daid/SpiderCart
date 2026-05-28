cmake_minimum_required(VERSION 3.14)
project(SpiderCartLoader LANGUAGES NONE)

find_package(Python3)

set(GB_HLA_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../GB.HLA/" CACHE FILEPATH "Path to GB.HLA installation")

add_custom_target(Loader.gb ALL Python3::Interpreter ${GB_HLA_DIR}/main.py ${CMAKE_CURRENT_LIST_DIR}/main.asm --output Loader.gb --symbols Loader.sym)
