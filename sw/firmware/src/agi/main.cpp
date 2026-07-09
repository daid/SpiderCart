#include "agi.h"
#include <stdio.h>

int main(int argc, char** argv)
{
    AGI::Engine engine;
    auto logic0 = engine.res_logic.load(0);
    engine.step();
    engine.step();

    for(int y=0; y<168; y+=2) {
        for(int x=0; x<160; x++) {
            auto c0 = engine.screen.display.buffer[x + y * 160];
            auto c1 = engine.screen.display.buffer[x + y * 160 + 160];
            printf("\x1B[38:5:%dm\x1B[48:5:%dm\u2580", c0, c1);
        }
        printf("\x1B[38:5:7m\x1B[48:5:0m\n");
    }
    
    return 0;
}
