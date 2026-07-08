#pragma once

#include "logic.h"
#include "resources.h"
#include <bitset>

namespace AGI {

class Engine
{
public:
    Engine();
    
    void step();

    ResourceManager<LogicResource> res_logic{"LOGDIR"};

    uint8_t var[256];
    std::bitset<256> flag;

    static constexpr int FLAG_ROOM_FIRST_TIME = 5;
    static constexpr int FLAG_LOGIC0_FIRST_TIME = 11;
};

}