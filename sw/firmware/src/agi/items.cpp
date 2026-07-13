#include "items.h"
#include "filesystem.h"
#include <stdio.h>
#include <string.h>


namespace AGI {

Items::Items()
{
    File object_file("OBJECT");
    data_size = object_file.size();
    data = (uint8_t*)malloc(data_size);
    object_file.read(data, data_size);

    if ((data[0] | (data[1] << 8)) >= data_size) {
        for(int n=0; n<data_size; n++)
            data[n] ^= "Avis Durgan"[n % 11];
    }
}

int Items::count()
{
    return (data[0] | (data[1] << 8)) / 3;
}

int Items::startRoom(int index)
{
    return data[5+index*3];
}

const char* Items::name(int index)
{
    int so = data[3+index*3] | (data[4+index*3] << 8);
    return reinterpret_cast<const char*>(data + 3 + so);
}

}