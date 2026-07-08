#pragma once

namespace AGI {

class Object // viewable ingame object, not to be confused with an inventory item (which are stored in the OBJECT file and named Item)
{
public:
    int16_t x;
    int16_t y;
};

}