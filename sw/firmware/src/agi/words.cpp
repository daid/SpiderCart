#include "words.h"
#include "filesystem.h"
#include <algorithm>
#include <stdio.h>
#include <ctype.h>
#include <string.h>


namespace AGI {

Words::Words()
{
    File words_file("WORDS.TOK");
    data_size = words_file.size();
    data = (uint8_t*)malloc(data_size);
    words_file.read(data, data_size);
    auto idx = 26*2;
    //Decrypt all the word data.
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

int Words::getWordID(const char* word)
{
    if (word[0] < 'a' || word[0] > 'z') return -1;
    auto offset = (data[(word[0]-'a')*2] << 8) | data[(word[0]-'a')*2+1];
    if (offset < 26*2) return -1;
    int match_length = 0;
    while(offset < data_size - 1) {
        int word_length = data[offset];
        match_length = std::min(match_length, word_length);
        if (word_length == 0 && (data[offset+1] & 0x7F) > word[0]) return -1;
        do {
            offset++;
            if (match_length == word_length) {
                if ((data[offset] & 0x7F) == word[match_length]) match_length++;
            }
            word_length++;
        } while(!(data[offset] & 0x80));
        if (word_length == match_length && word[word_length] == '\0')
            return (data[offset+1] << 8) | data[offset+2];
        offset += 3;
    }

    return -1;
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
