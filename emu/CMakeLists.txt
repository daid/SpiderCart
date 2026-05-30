cmake_minimum_required(VERSION 3.11)
project(SpiderCartEmu)
add_subdirectory(binjgb)
add_subdirectory(../sw/loader loader)

target_sources(binjgb PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/mbc.cpp)
target_sources(binjgb-debugger PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/mbc.cpp)
