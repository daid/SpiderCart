#include "picture.h"
#include "screen.h"


namespace AGI {

void PictureResource::draw(Screen& screen)
{
    size_t idx = 0;
    int pen_style = 0;
    int draw_color = -1;
    int prio_color = -1;
    while(data[idx] != 0xFF) {
        switch(data[idx++]) {
        case 0xF0: draw_color = data[idx++]; break;
        case 0xF1: draw_color = -1; break;
        case 0xF2: prio_color = data[idx++]; break;
        case 0xF3: prio_color = -1; break;
        case 0xF4:
            {
                uint8_t x0 = data[idx++];
                uint8_t y0 = data[idx++];
                uint8_t x1 = x0;
                while(data[idx] < 0xF0) {
                    uint8_t y1 = data[idx++];
                    if (draw_color >= 0) screen.display.line(x0, y0, x1, y1, draw_color);
                    if (prio_color >= 0) screen.priority.line(x0, y0, x1, y1, prio_color);
                    y0 = y1;
                    if (data[idx] < 0xF0) {
                        x1 = data[idx++];
                        if (draw_color >= 0) screen.display.line(x0, y0, x1, y1, draw_color);
                        if (prio_color >= 0) screen.priority.line(x0, y0, x1, y1, prio_color);
                        x0 = x1;
                    }
                }
            }
            break;
        case 0xF5:
            {
                uint8_t x0 = data[idx++];
                uint8_t y0 = data[idx++];
                uint8_t y1 = y0;
                while(data[idx] < 0xF0) {
                    uint8_t x1 = data[idx++];
                    if (draw_color >= 0) screen.display.line(x0, y0, x1, y1, draw_color);
                    if (prio_color >= 0) screen.priority.line(x0, y0, x1, y1, prio_color);
                    x0 = x1;
                    if (data[idx] < 0xF0) {
                        y1 = data[idx++];
                        if (draw_color >= 0) screen.display.line(x0, y0, x1, y1, draw_color);
                        if (prio_color >= 0) screen.priority.line(x0, y0, x1, y1, prio_color);
                        y0 = y1;
                    }
                }
            }
            break;
        case 0xF6:
            {
                uint8_t x0 = data[idx++];
                uint8_t y0 = data[idx++];
                while(data[idx] < 0xF0) {
                    uint8_t x1 = data[idx++];
                    uint8_t y1 = data[idx++];
                    if (draw_color >= 0) screen.display.line(x0, y0, x1, y1, draw_color);
                    if (prio_color >= 0) screen.priority.line(x0, y0, x1, y1, prio_color);
                    x0 = x1;
                    y0 = y1;
                }
            }
            break;
        case 0xF7:
            {
                int x0 = data[idx++];
                int y0 = data[idx++];
                while (data[idx] < 0xF0) {
                    auto xy = data[idx++];
                    int x1, y1;
                    if (xy & 0x80)
                        x1 = x0 - ((xy >> 4) & 0x07);
                    else
                        x1 = x0 + ((xy >> 4) & 0x07);
                    if (xy & 0x08)
                        y1 = y0 - (xy & 0x07);
                    else
                        y1 = y0 + (xy & 0x07);
                    if (draw_color >= 0) screen.display.line(x0, y0, x1, y1, draw_color);
                    if (prio_color >= 0) screen.priority.line(x0, y0, x1, y1, prio_color);
                    x0 = x1;
                    y0 = y1;
                }
            }
            break;
        case 0xF8:
            while(data[idx] < 0xF0) {
                auto x = data[idx++];
                auto y = data[idx++];
                if (draw_color >= 0 && prio_color >= 0) screen.fill(x, y, 15, draw_color, prio_color);
                else if (prio_color >= 0 && prio_color != 4) screen.priority.fill(x, y, 4, prio_color);
                else if (draw_color >= 0 && draw_color != 15) screen.display.fill(x, y, 15, draw_color);
            }
            break;
        case 0xF9:
            pen_style = data[idx++];
            break;
        case 0xFA:
            //TODO: Pen style handling and pattern handling not yet implemented.
            //      While the effect so far seems limited, this has the potential to greatly corrupt certain pictures.
            if (pen_style & 0x27)
                printf("Warning, not properly handling pen style: %02x\n", pen_style);
            while(data[idx] < 0xF0) {
                if (pen_style & 0x20)
                    uint8_t pattern = data[idx++];
                int x = data[idx++];
                int y = data[idx++];
                if (draw_color >= 0) screen.display.set(x, y, draw_color);
                if (prio_color >= 0) screen.priority.set(x, y, prio_color);
            }
            break;
        default:
            printf("Unknown picture opcode: %02X\n", data[idx-1]);
            return;
        }
    }
}

}