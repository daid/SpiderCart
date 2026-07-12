#include "words.h"
#include "filesystem.h"
#include <stdio.h>


namespace AGI {

Words::Words()
{
    File words_file("WORDS.TOK");
    data_size = words_file.size();
    data = (uint8_t*)malloc(data_size);
    words_file.read(data, data_size);
    auto idx = 26*2;
    while(idx < data_size) {
        idx += 1; //skip "copy prev"
        if (idx >= data_size) break; // File ends with a single 00 byte.
        while(true) {
            data[idx] = data[idx] ^ 0x7F;
            if (data[idx] & 0x80) break;
            idx++;
        }
        idx += 1;
        idx += 2; //skip "word ID"
    }
}

void Words::getWord(int id, char* buffer, size_t max_length)
{
    //Find the shorted word by going through the list twice, first to find length of each and then to find a shortest.
    size_t max_word_length = 0xFFFF;
    auto idx = 26*2;
    while(idx < data_size) {
        auto ptr = buffer + data[idx];
        idx += 1; //skip "copy prev"
        if (idx >= data_size) break; // File ends with a single 00 byte.
        while(true) {
            if (ptr < buffer + max_length)
                *ptr++ = data[idx] & 0x7F;
            if (data[idx] & 0x80) break;
            idx++;
        }
        idx += 1;
        auto this_id = (data[idx] << 8) | data[idx + 1];
        if (this_id == id) {
            if (ptr < buffer + max_length)
                *ptr = 0;
            else
                buffer[max_length - 1] = 0;
            if (strlen(buffer) < max_word_length)
                max_word_length = strlen(buffer);
        }
        idx += 2; //skip "word ID"
    }

    idx = 26*2;
    while(idx < data_size) {
        auto ptr = buffer + data[idx];
        idx += 1; //skip "copy prev"
        if (idx >= data_size) break; // File ends with a single 00 byte.
        while(true) {
            if (ptr < buffer + max_length)
                *ptr++ = data[idx] & 0x7F;
            if (data[idx] & 0x80) break;
            idx++;
        }
        idx += 1;
        auto this_id = (data[idx] << 8) | data[idx + 1];
        if (this_id == id) {
            if (ptr < buffer + max_length)
                *ptr = 0;
            else
                buffer[max_length - 1] = 0;
            if (strlen(buffer) == max_word_length)
                return;
        }
        idx += 2; //skip "word ID"
    }
    buffer[0] = 0;
}

}