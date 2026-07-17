#pragma once

#include <stdint.h>
#include <stdlib.h>
#include "view.h"

namespace AGI {

class ScreenBuffer
{
public:
    void clear(uint8_t color);
    void set(int x, int y, uint8_t color);
    void line(int x0, int y0, int x1, int y1, uint8_t color);
    void fill(int x, int y, uint8_t from_color, uint8_t to_color);

    static constexpr int WIDTH = 160;
    static constexpr int HEIGHT = 168;
    uint8_t buffer[WIDTH*HEIGHT];
};

class Screen
{
public:
    void clear();
    void fill(int x, int y, uint8_t from_color, uint8_t display_color, uint8_t priority_color);
    void drawView(const ViewResource::Info& view, int x, int y, int prio, int margin);
    void drawText(int x, int y, const char* str);
    int getPrioValue(int x, int y);
    uint32_t getPrioBits(int x, int y, int width);

    ScreenBuffer display;
    ScreenBuffer priority;
};

}