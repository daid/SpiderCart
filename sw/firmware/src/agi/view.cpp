#include "view.h"


namespace AGI {

int ViewResource::loopCount()
{
    return data[2];
}

int ViewResource::celCount(int loop)
{
    auto offset = u16(5 + loop * 2);
    return data[offset];
}

ViewResource::Info ViewResource::info(int loop, int cel)
{
    auto offset = u16(5 + loop * 2);
    offset += u16(offset + 1 + cel * 2);
    return {
        data[offset + 0],
        data[offset + 1],
        (loop != ((data[offset + 2] >> 4) & 0x07)) ? !!(data[offset + 2] & 0x80) : false,
        data[offset + 2] & 0x0F,
        &data[offset + 3]
    };
}

}