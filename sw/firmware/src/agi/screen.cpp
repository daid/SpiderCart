#include "screen.h"
#include <string.h>
#include <math.h>
#include <stdio.h>

namespace AGI {

void ScreenBuffer::clear(uint8_t color)
{
    memset(buffer, color, sizeof(buffer));
}

void ScreenBuffer::set(int x, int y, uint8_t color)
{
    buffer[x + y * WIDTH] = color;
}

void ScreenBuffer::line(int x0, int y0, int x1, int y1, uint8_t color)
{
    auto dx = abs(x1 - x0);
    auto sx = x0 < x1 ? 1 : -1;
    auto dy = -abs(y1 - y0);
    auto sy = y0 < y1 ? 1 : -1;
    auto error = dx + dy;
    while(true) {
        buffer[x0 + y0 * WIDTH] = color;
        auto e2 = 2 * error;
        if (e2 >= dy) {
            if (x0 == x1)
                break;
            error = error + dy;
            x0 = x0 + sx;
        }
        if (e2 <= dx) {
            if (y0 == y1)
                break;
            error = error + dx;
            y0 = y0 + sy;
        }
    }
}

void ScreenBuffer::fill(int x, int y, uint8_t from_color, uint8_t to_color)
{
    auto line_ptr = buffer + y * WIDTH;
    int x0 = x;
    int x1 = x;
    while(x0 >= 0 && line_ptr[x0] == from_color) x0--;
    while(x1 < WIDTH && line_ptr[x1] == from_color) x1++;
    x0 += 1;
    for(x=x0; x<x1; x++)
        line_ptr[x] = to_color;
    line_ptr -= WIDTH;
    if (y > 0) {
        for(x=x0; x<x1; x++)
            if (line_ptr[x] == from_color)
                fill(x, y-1, from_color, to_color);
    }
    line_ptr += WIDTH * 2;
    if (y < HEIGHT - 1) {
        for(x=x0; x<x1; x++)
            if (line_ptr[x] == from_color)
                fill(x, y+1, from_color, to_color);
    }
}

void Screen::clear()
{
    display.clear(15);
    priority.clear(4);
}

}