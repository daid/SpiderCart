#pragma once

#include <stdint.h>
#include <stdlib.h>

namespace AGI {

class Items
{
public:
    Items();

    int count();
    int startRoom(int index);
    const char* name(int index);

private:
    uint8_t* data;
    size_t data_size;
};

}