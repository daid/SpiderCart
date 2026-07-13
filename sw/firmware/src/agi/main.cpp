#include "agi.h"
#include <stdio.h>
#include <SDL.h>
#include <vector>
#include <algorithm>

static constexpr int WIDTH = 160*2;
static constexpr int HEIGHT = 168;

#define RGB32(b, g, r) (0xFF000000 | (r << 2) | (g << 10) | (b << 18))
static const uint32_t COLORS[] = {
    RGB32(0x00, 0x00, 0x00),
    RGB32(0x00, 0x00, 0x2A),
    RGB32(0x00, 0x2A, 0x00),
    RGB32(0x00, 0x2A, 0x2A),
    RGB32(0x2A, 0x00, 0x00),
    RGB32(0x2A, 0x00, 0x2A),
    RGB32(0x2A, 0x15, 0x00),
    RGB32(0x2A, 0x2A, 0x2A),
    RGB32(0x15, 0x15, 0x15),
    RGB32(0x15, 0x15, 0x3F),
    RGB32(0x15, 0x3F, 0x15),
    RGB32(0x15, 0x3F, 0x3F),
    RGB32(0x3F, 0x15, 0x15),
    RGB32(0x3F, 0x15, 0x3F),
    RGB32(0x3F, 0x3F, 0x15),
    RGB32(0x3F, 0x3F, 0x3F),
};

namespace AGI {
extern const uint8_t fontData[];
}

void drawBox(uint32_t* pixels, int x, int y, int w, int h, uint32_t c)
{
    for(int y0=0; y0<h; y0++)
        for(int x0=0; x0<w; x0++)
            pixels[(x+x0)+(y+y0)*WIDTH] = c;
}

void drawString(uint32_t* pixels, int x, int y, const char* str, size_t length, uint32_t c)
{
    while(*str && length) {
        const uint8_t* ptr = AGI::fontData;
        if (*str >= 32 && *str < 128) {
            ptr = AGI::fontData + (*str - 32) * 4;
        }
        for(int px=0; px<4; px++) {
            for(int py=0; py<8; py++)
                if ((*ptr) & (1 << py))
                    pixels[x + (y+py) * WIDTH] = c;
            x++;
            ptr++;
        }
        if (x >= 160 - 3) { x = 0; y += 8; }
        str++;
        length--;
    }
}

void drawMessage(uint32_t* pixels, const char* str)
{
    int line_count = (strlen(str) + 39) / 40;
    drawBox(pixels, 0, 84 - line_count * 4 - 2, 160, line_count * 8 + 4, COLORS[15]);
    drawString(pixels, 0, 84 - line_count * 4, str, strlen(str), COLORS[0]);
}

enum class TextInputState
{
    None,
    GatherSaidList,
    InputWords,
    Execute
} text_input_state = TextInputState::None;

int main(int argc, char** argv)
{
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) { printf("SDL_Init failure: %s\n", SDL_GetError()); return 1; }
    auto window = SDL_CreateWindow("AGI", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH*4, HEIGHT*4, SDL_WINDOW_SHOWN);
    if (!window) { printf("SDL_CreateWindow failure: %s\n", SDL_GetError()); return 1; }
    auto winSurface = SDL_GetWindowSurface(window);
    auto surface = SDL_CreateRGBSurface(0, WIDTH, HEIGHT, 32, 0,0,0,0);

    AGI::Engine engine;
    auto logic0 = engine.res_logic.load(0);
    bool running = true;
    bool stepping = true;
    while(running) {
        if (engine.message_list_size == 0 && stepping) {
            auto res = engine.step();
            //printf("################## STEP: %d ##################\n", res);
            if (res < 0) stepping = false;
        }

        SDL_Event e;
        while(SDL_PollEvent(&e)) {
            switch(e.type) {
            case SDL_QUIT:
                running = false;
                break;
            case SDL_KEYDOWN:
                if (e.key.keysym.sym == SDLK_ESCAPE) running = false;
                if (e.key.keysym.sym == SDLK_LEFT) engine.var[AGI::Engine::VAR_PLAYER_DIRECTION] = engine.var[AGI::Engine::VAR_PLAYER_DIRECTION] == 7 ? 0 : 7;
                if (e.key.keysym.sym == SDLK_RIGHT) engine.var[AGI::Engine::VAR_PLAYER_DIRECTION] = engine.var[AGI::Engine::VAR_PLAYER_DIRECTION] == 3 ? 0 : 3;
                if (e.key.keysym.sym == SDLK_UP) engine.var[AGI::Engine::VAR_PLAYER_DIRECTION] = engine.var[AGI::Engine::VAR_PLAYER_DIRECTION] == 1 ? 0 : 1;
                if (e.key.keysym.sym == SDLK_DOWN) engine.var[AGI::Engine::VAR_PLAYER_DIRECTION] = engine.var[AGI::Engine::VAR_PLAYER_DIRECTION] == 5 ? 0 : 5;
                if (e.key.keysym.sym == 's') stepping = true;
                if (engine.message_list_size > 0) {
                    engine.message_list_size -= 1;
                    for(int n=0; n<engine.message_list_size; n++)
                        engine.message_list[n] = engine.message_list[n+1];
                } else {
                    engine.any_key_pressed = true;
                }
                if (e.key.keysym.sym == SDLK_RETURN) {
                    // qsort(engine.said_options, engine.said_options_size, sizeof(uint16_t), [](const void* a, const void* b) -> int {
                    //     char buffer_a[32];
                    //     char buffer_b[32];
                    //     AGI::Engine::instance->words.getWord(*reinterpret_cast<const uint16_t*>(a), buffer_a, sizeof(buffer_a));
                    //     AGI::Engine::instance->words.getWord(*reinterpret_cast<const uint16_t*>(b), buffer_b, sizeof(buffer_b));
                    //     return strcmp(buffer_a, buffer_b);
                    // });
                    // printf("SAY: ");
                    // for(int n=0; n<engine.said_list_size; n++) {
                    //     char buffer[32];
                    //     engine.words.getWord(engine.said_list[n], buffer, sizeof(buffer));
                    //     printf("%s ", buffer);
                    // }
                    // printf("\n");
                    // for(int n=0; n<engine.said_options_size; n++) {
                    //     char buffer[32];
                    //     engine.words.getWord(engine.said_options[n], buffer, sizeof(buffer));
                    //     printf(" %c:%s\n", 'a' + n, buffer);
                    // }

                    // if (engine.said_options_size == 0) {
                        engine.flag[AGI::Engine::FLAG_TEXT_INPUT_DONE] = true;
                    // }
                }
                if (e.key.keysym.sym >= 'a' && e.key.keysym.sym < 'a' + engine.said_options_size) {
                    qsort(engine.said_options, engine.said_options_size, sizeof(uint16_t), [](const void* a, const void* b) -> int {
                        char buffer_a[32];
                        char buffer_b[32];
                        AGI::Engine::instance->words.getWord(*reinterpret_cast<const uint16_t*>(a), buffer_a, sizeof(buffer_a));
                        AGI::Engine::instance->words.getWord(*reinterpret_cast<const uint16_t*>(b), buffer_b, sizeof(buffer_b));
                        return strcmp(buffer_a, buffer_b);
                    });

                    engine.said_list[engine.said_list_size++] = engine.said_options[e.key.keysym.sym - 'a'];
                }
                break;
            }
        }

        SDL_LockSurface(surface);
        auto pixels = reinterpret_cast<uint32_t*>(surface->pixels);
        for(int y=0; y<168; y++) {
            for(int x=0; x<160; x++) {
                auto c = engine.screen.display.buffer[x + y * 160];
                pixels[x + y * WIDTH] = COLORS[c];
                c = engine.screen.priority.buffer[x + y * 160];
                pixels[x + 160 + y * WIDTH] = COLORS[c];
            }
        }
        std::vector<AGI::Object*> draw_list;
        for(auto& obj : engine.object) {
            if (!(obj.flags & AGI::Object::flag_anim)) continue;
            if (!(obj.flags & AGI::Object::flag_draw)) continue;
            draw_list.push_back(&obj);
        }
        std::sort(draw_list.begin(), draw_list.end(), [](const auto a, const auto b) -> bool {
            return a->y < b->y;
        });
        for(auto obj : draw_list) {
            if (!(obj->flags & AGI::Object::flag_anim)) continue;
            if (obj->flags & AGI::Object::flag_draw) {
                auto info = engine.res_view[obj->view]->info(obj->loop, obj->cel);
                for(int y=0; y<info.height; y++) {
                    for(int x=info.mirror?info.width-1:0;;) {
                        auto chunk = *info.data++;
                        if (!chunk) break;
                        for(int n=0; n<(chunk&0x0F); n++) {
                            if ((chunk >> 4) != info.transparent) {
                                auto px = obj->x+x;
                                auto py = obj->y+y-info.height+1;
                                if (px >= 0 && px < 160 && py >= 0 && py < 168) {
                                    if (obj->priority >= engine.screen.getPrioValue(px, py))
                                        pixels[px + py * WIDTH] = COLORS[chunk >> 4];
                                    pixels[px + 160 + py * WIDTH] = COLORS[chunk >> 4];

                                }
                            }
                            if (info.mirror) x--; else x++;
                        }
                    }
                }
                char buf[32];
                sprintf(buf, "o%d", obj->objIndex());
                drawString(pixels + 160, obj->x, obj->y - info.height + 1, buf, strlen(buf), COLORS[15]);
            }
        }

        if (engine.message_list_size > 0) {
            auto str = engine.message_list[0].logic->str(engine.message_list[0].index);
            drawMessage(pixels, str);
        }

        SDL_UnlockSurface(surface);

        SDL_BlitScaled(surface, nullptr, winSurface, nullptr);
        SDL_UpdateWindowSurface(window);
        SDL_Delay(50);
    }
    
    return 0;
}
