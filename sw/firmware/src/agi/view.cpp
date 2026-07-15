#include "view.h"
#include <assert.h>


namespace AGI {

int ViewResource::loopCount()
{
    return data[2];
}

const char* ViewResource::description()
{
    auto offset = data[3] | (data[4] << 8);
    if (offset > 0) {
        return (const char*)data + offset;
    }
    return "";
}

int ViewResource::celCount(int loop)
{
    assert(loop < loopCount());
    auto offset = u16(5 + loop * 2);
    return data[offset];
}

ViewResource::Info ViewResource::info(int loop, int cel)
{
    assert(loop < loopCount());
    auto offset = u16(5 + loop * 2);
    assert(cel < data[offset]);
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