#pragma once

#include "resources.h"


namespace AGI {

class ViewResource : public Resource
{
public:
    ViewResource(uint8_t* data, size_t size) : Resource(data, size) {}

    int loopCount();
    int celCount(int loop);
    struct Info {
        int width, height;
        bool mirror;
        int transparent;
        const uint8_t* data;
    };
    Info info(int loop, int cel);
};

}