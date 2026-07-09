#pragma once

#include "resources.h"


namespace AGI {

class Screen;
class PictureResource : public Resource
{
public:
    PictureResource(uint8_t* data, size_t size) : Resource(data, size) {}

    void draw(Screen& screen);
};

}