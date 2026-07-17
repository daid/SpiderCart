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

void Screen::fill(int x, int y, uint8_t from_color, uint8_t display_color, uint8_t priority_color)
{
    if (display_color == from_color) return;
    auto line_ptr = display.buffer + y * display.WIDTH;
    int x0 = x;
    int x1 = x;
    while(x0 >= 0 && line_ptr[x0] == from_color) x0--;
    while(x1 < display.WIDTH && line_ptr[x1] == from_color) x1++;
    x0 += 1;
    for(x=x0; x<x1; x++)
        line_ptr[x] = display_color;
    for(x=x0; x<x1; x++)
        priority.buffer[x + y * priority.WIDTH] = priority_color;
    line_ptr -= display.WIDTH;
    if (y > 0) {
        for(x=x0; x<x1; x++)
            if (line_ptr[x] == from_color)
                fill(x, y-1, from_color, display_color, priority_color);
    }
    line_ptr += display.WIDTH * 2;
    if (y < display.HEIGHT - 1) {
        for(x=x0; x<x1; x++)
            if (line_ptr[x] == from_color)
                fill(x, y+1, from_color, display_color, priority_color);
    }
}

void Screen::drawView(const ViewResource::Info& info, int vx, int vy, int prio, int margin)
{
    //TODO: if prio == 0, prio = autoprio

    auto ptr = info.data;
    for(int y=0; y<info.height; y++) {
        for(int x=info.mirror?info.width-1:0;;) {
            auto chunk = *ptr++;
            if (!chunk) break;
            for(int n=0; n<(chunk&0x0F); n++) {
                if ((chunk >> 4) != info.transparent) {
                    auto px = vx+x;
                    auto py = vy+y-info.height+1;
                    if (px >= 0 && px < 160 && py >= 0 && py < 168) {
                        display.buffer[px + py * 160] = chunk >> 4;
                        if (priority.buffer[px + py * 160] >= 4)
                            priority.buffer[px + py * 160] = prio;
                    }
                }
                if (info.mirror) x--; else x++;
            }
        }
    }
    if (margin < 3) {
        auto y0 = vy/12*12+1;
        auto y1 = vy;
        for(int n=y0; n<=y1; n++) {
            if (priority.buffer[vx + n * 160] >= 4) priority.buffer[vx + n * 160] = margin;
            if (priority.buffer[vx + info.width-1 + n * 160] >= 4) priority.buffer[vx + info.width-1 + n * 160] = margin;
        }
        for(int n=0; n<info.width; n++) {
            if (priority.buffer[vx + n + y0 * 160] >= 4) priority.buffer[vx + n + y0 * 160] = margin;
            if (priority.buffer[vx + n + y1 * 160] >= 4) priority.buffer[vx + n + y1 * 160] = margin;
        }
    }
}

extern const uint8_t fontData[];
const uint8_t fontData[] = {
    #include "font.inc"
};

void Screen::drawText(int x, int y, const char* str)
{
    if (y < 0) return;
    while(*str) {
        if (y > 160) return;
        const uint8_t* ptr = fontData;
        if (*str >= 32 && *str < 128) {
            ptr = fontData + (*str - 32) * 4;
        }
        for(int px=0; px<4; px++) {
            for(int py=0; py<8; py++)
                display.buffer[x + (y+py) * 160] = (*ptr) & (1 << py) ? 15 : 0;
            x++;
            ptr++;
        }
        str++;
    }
}

int Screen::getPrioValue(int x, int y)
{
    while(y < 168 && priority.buffer[x + y * 160] < 3)
        y++;
    if (y == 168) return 4;
    return priority.buffer[x + y * 160];
}

uint32_t Screen::getPrioBits(int x, int y, int width)
{
    uint32_t res = 0;
    for(int n=0; n<width; n++)
        res |= 1 << priority.buffer[(x+n) + y * 160];
    return res;
}

}