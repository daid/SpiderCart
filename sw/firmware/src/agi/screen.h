#pragma once

#include <stdint.h>
#include <stdlib.h>

namespace AGI {

class ScreenBuffer
{
public:
    void clear(uint8_t color);
    void set(int x, int y, uint8_t color);
    void line(int x0, int y0, int x1, int y1, uint8_t color);
    void fill(int x, int y, uint8_t from_color, uint8_t to_color);

    static constexpr size_t WIDTH = 160;
    static constexpr size_t HEIGHT = 168;
    uint8_t buffer[WIDTH*HEIGHT];
};

class Screen
{
public:
    void clear();

    ScreenBuffer display;
    ScreenBuffer priority;
};

}