#include "logic.h"
#include <stdio.h>


namespace AGI {

LogicResource::LogicResource(uint8_t* data, size_t size)
: Resource(data, size)
{
    // Lookup the string table and unencrypt it.
    size_t read_idx = 0;
    read_idx = u16(read_idx);
    read_idx += 2;
    logic_size = read_idx;
    string_count = data[read_idx++];
    string_table_idx = read_idx;
    read_idx += 2 * string_count;
    read_idx += 2; //There seems to be 1 extra value in this table...
    int n = 0;
    while(read_idx < size) {
        data[read_idx++] ^= "Avis Durgan"[n++];
        if (n == 11) n = 0;
    }
}

const char* LogicResource::str(unsigned int index)
{
    if (index > string_count) return "ERR";
    size_t offset = u16(string_table_idx + index * 2);
    return (const char*)(data + string_table_idx + offset);
}

}