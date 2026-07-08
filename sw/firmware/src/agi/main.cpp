#include "agi.h"
#include <stdio.h>

int main(int argc, char** argv)
{
    AGI::Engine engine;
    auto logic0 = engine.res_logic.load(0);
    engine.step();
    engine.step();
    
    return 0;
}
