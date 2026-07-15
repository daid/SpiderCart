#pragma once

#include <stdint.h>
#include <stdlib.h>

namespace AGI {

class Words
{
public:
    Words();

    int getWordID(const char* word);
    void getWord(int id, char* buffer, size_t max_length);
private:
    uint8_t* data;
    size_t data_size;
};

}