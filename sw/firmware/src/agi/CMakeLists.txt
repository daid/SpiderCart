cmake_minimum_required(VERSION 3.11)
project(AGI)

add_executable(AGI
    main.cpp
    agi.cpp
    logic.cpp
    picture.cpp
    screen.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/../filesystem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../emu/src/ff.cpp
)
target_include_directories(AGI PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../inc)