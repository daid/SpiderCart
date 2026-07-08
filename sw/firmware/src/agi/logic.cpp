#include "logic.h"
#include <stdio.h>


namespace AGI {

LogicResource::LogicResource(uint8_t* data, size_t size)
: Resource(data, size)
{
    // Lookup the string table and unencrypt it.
    size_t read_idx = 0;
    read_idx = data[read_idx] | (data[read_idx + 1] << 8);
    read_idx += 2;
    logic_size = read_idx;
    string_count = data[read_idx++];
    read_idx += 2; // skip some pointer to the end of the string table data (which is the end of the resource as well)
    string_table_idx = read_idx;
    read_idx += 2 * string_count;
    int n = 0;
    while(read_idx < size) {
        data[read_idx++] ^= "Avis Durgan"[n++];
        if (n == 11) n = 0;
    }
}

const char* LogicResource::str(int index)
{
    if (index >= string_count) return "ERR";
    size_t offset = data[string_table_idx + index * 2] | (data[string_table_idx + index * 2 + 1] << 8);
    return (const char*)(data + string_table_idx + offset - 2);
}

}