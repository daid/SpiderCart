#pragma once

#include "words.h"
#include "items.h"
#include "logic.h"
#include "resources.h"
#include "view.h"
#include "picture.h"
#include "object.h"
#include "screen.h"
#include <bitset>
#include <array>

namespace AGI {

class Engine
{
public:
    Engine();
    static Engine* instance;
    
    int step();
    int runLogic(LogicResource* logic);
    bool checkSaid(int amount, uint8_t* data);
    void fillSaidOptions();
    void fillSaidOptions(LogicResource* logic);
    void fillSaidOptions(int amount, uint8_t* data);

    Words words;
    Items items;
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

    bool player_control = true;
    bool input_enabled = true;
    bool show_status = true;
    int horizon = 36;
    bool block_active = false;
    int block_x0, block_y0, block_x1, block_y1;

    struct Message {
        LogicResource* logic;
        int index;
    };
    std::array<Message, 16> message_list;
    int message_list_size = 0;

    /* input */
    bool any_key_pressed = false;
    uint16_t said_list[16] = {0};
    size_t said_list_size = 0;
    uint16_t said_options[64] = {0};
    size_t said_options_size = 0;

    static constexpr int VAR_CURRENT_ROOM = 0;
    static constexpr int VAR_PREV_ROOM = 1;
    static constexpr int VAR_PLAYER_BORDER_TOUCH = 2;
    static constexpr int VAR_SCORE = 3;
    static constexpr int VAR_OTHER_BORDER_TOUCH_OBJ = 4;
    static constexpr int VAR_OTHER_BORDER_TOUCH = 5;
    static constexpr int VAR_PLAYER_DIRECTION = 6;
    static constexpr int VAR_PLAYER_VIEW = 16;

    static constexpr int FLAG_PLAYER_ON_WATER = 0;
    static constexpr int FLAG_TEXT_INPUT_DONE = 2;
    static constexpr int FLAG_PLAYER_TOUCHED_TRIGGER = 3;
    static constexpr int FLAG_SAID_ACCEPTED_INPUT = 4;
    static constexpr int FLAG_ROOM_FIRST_TIME = 5;
    static constexpr int FLAG_RESTART_GAME = 6;
    static constexpr int FLAG_LOGIC0_FIRST_TIME = 11;
};

}