cmake_minimum_required(VERSION 3.11)
project(AGI)

find_package(sdl2 REQUIRED)

add_executable(AGI
    main.cpp
    agi.cpp
    words.cpp
    logic.cpp
    picture.cpp
    screen.cpp
    view.cpp
    object.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/../filesystem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../emu/src/ff.cpp
)
target_include_directories(AGI PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../inc)
target_link_libraries(AGI SDL2::SDL2main SDL2::SDL2)