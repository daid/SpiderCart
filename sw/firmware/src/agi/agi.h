#pragma once

#include "logic.h"
#include "resources.h"
#include "view.h"
#include "picture.h"
#include "object.h"
#include "screen.h"
#include <bitset>

namespace AGI {

class Engine
{
public:
    Engine();
    
    void step();
    int runLogic(LogicResource* logic);

    ResourceManager<LogicResource> res_logic{"LOGDIR"};
    ResourceManager<ViewResource> res_view{"VIEWDIR"};
    ResourceManager<PictureResource> res_picture{"PICDIR"};

    Screen screen;

    uint8_t var[256] = {0};
    std::bitset<256> flag;
    std::string str[12];
    Object object[256];
    uint8_t item_room[256] = {0};
    int new_room_nr = -1;

    bool input_enabled = true;
    int horizon = 36;

    static constexpr int FLAG_ROOM_FIRST_TIME = 5;
    static constexpr int FLAG_LOGIC0_FIRST_TIME = 11;
};

}