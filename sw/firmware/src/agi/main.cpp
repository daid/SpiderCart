#include "agi.h"
#include <stdio.h>
#include <SDL.h>

static constexpr int WIDTH = 160;
static constexpr int HEIGHT = 168;

#define RGB(b, g, r) (0xFF000000 | (r << 2) | (g << 10) | (b << 18))
static const uint32_t COLORS[] = {
    RGB(0x00, 0x00, 0x00),
    RGB(0x00, 0x00, 0x2A),
    RGB(0x00, 0x2A, 0x00),
    RGB(0x00, 0x2A, 0x2A),
    RGB(0x2A, 0x00, 0x00),
    RGB(0x2A, 0x00, 0x2A),
    RGB(0x2A, 0x15, 0x00),
    RGB(0x2A, 0x2A, 0x2A),
    RGB(0x15, 0x15, 0x15),
    RGB(0x15, 0x15, 0x3F),
    RGB(0x15, 0x3F, 0x15),
    RGB(0x15, 0x3F, 0x3F),
    RGB(0x3F, 0x15, 0x15),
    RGB(0x3F, 0x15, 0x3F),
    RGB(0x3F, 0x3F, 0x15),
    RGB(0x3F, 0x3F, 0x3F),
};

int main(int argc, char** argv)
{
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) { printf("SDL_Init failure: %s\n", SDL_GetError()); return 1; }
    auto window = SDL_CreateWindow("AGI", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH*3, HEIGHT*3, SDL_WINDOW_SHOWN);
    if (!window) { printf("SDL_CreateWindow failure: %s\n", SDL_GetError()); return 1; }
    auto winSurface = SDL_GetWindowSurface(window);
    auto surface = SDL_CreateRGBSurface(0, WIDTH, HEIGHT, 32, 0,0,0,0);

    AGI::Engine engine;
    auto logic0 = engine.res_logic.load(0);
    bool running = true;
    bool stepping = true;
    while(running) {
        if (stepping) {
            auto res = engine.step();
            printf("################## STEP: %d ##################\n", res);
            if (res < 0) stepping = false;
            if (engine.res_view[engine.object[0].view]) {
                auto info = engine.res_view[engine.object[0].view]->info(engine.object[0].loop, engine.object[0].cel);
                printf("%d %d %d %d\n", engine.object[0].loop, engine.object[0].cel, info.width, info.height);
            }
        }

        SDL_Event e;
        while(SDL_PollEvent(&e)) {
            switch(e.type) {
            case SDL_QUIT:
                running = false;
                break;
            case SDL_KEYDOWN:
                if (e.key.keysym.sym == SDLK_ESCAPE) running = false;
                if (e.key.keysym.sym == SDLK_LEFT) engine.var[AGI::Engine::VAR_PLAYER_DIRECTION] = 7;
                if (e.key.keysym.sym == SDLK_RIGHT) engine.var[AGI::Engine::VAR_PLAYER_DIRECTION] = 3;
                if (e.key.keysym.sym == SDLK_UP) engine.var[AGI::Engine::VAR_PLAYER_DIRECTION] = 1;
                if (e.key.keysym.sym == SDLK_DOWN) engine.var[AGI::Engine::VAR_PLAYER_DIRECTION] = 5;
                if (e.key.keysym.sym == 's') stepping = true;
                engine.any_key_pressed = true;
                break;
            }
        }

        SDL_LockSurface(surface);
        auto pixels = reinterpret_cast<uint32_t*>(surface->pixels);
        for(int y=0; y<168; y++) {
            for(int x=0; x<160; x++) {
                auto c = engine.screen.display.buffer[x + y * 160];
                pixels[x + y * WIDTH] = COLORS[c];
            }
        }
        SDL_UnlockSurface(surface);
        for(auto& obj : engine.object) {
            if (!(obj.flags & AGI::Object::flag_anim)) continue;
            if (obj.flags & AGI::Object::flag_draw) {
                auto info = engine.res_view[obj.view]->info(obj.loop, obj.cel);
                for(int y=0; y<info.height; y++) {
                    for(int x=0;;) {
                        auto chunk = *info.data++;
                        if (!chunk) break;
                        for(int n=0; n<(chunk&0x0F); n++) {
                            if ((chunk >> 4) != info.transparent)
                                pixels[(obj.x+x) + (obj.y+y-info.height-1) * WIDTH] = COLORS[chunk >> 4];
                            x++;
                        }
                    }
                }
            }
        }

        SDL_BlitScaled(surface, nullptr, winSurface, nullptr);
        SDL_UpdateWindowSurface(window);
        SDL_Delay(50);
    }
    
    return 0;
}
