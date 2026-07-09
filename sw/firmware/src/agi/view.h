#pragma once

#include "resources.h"


namespace AGI {

class ViewResource : public Resource
{
public:
    ViewResource(uint8_t* data, size_t size) : Resource(data, size) {}
};

}