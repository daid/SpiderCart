#pragma once

namespace AGI {

class Object // viewable ingame object, not to be confused with an inventory item (which are stored in the OBJECT file and named Item)
{
public:
    int16_t x = 0;
    int16_t y = 0;
    
    uint8_t view = 0;
    uint8_t loop = 0;
    uint8_t cel = 0;

    uint8_t priority = 0;

    uint16_t flags = 0;
    static constexpr uint16_t flag_anim = 0x0001;
    static constexpr uint16_t flag_cycling = 0x0002;
    static constexpr uint16_t flag_observes = 0x0004;
};

}