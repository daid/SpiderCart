#pragma once
#include "filesystem.h"
#include <assert.h>
#include <stdio.h>
#include <stdint.h>


namespace AGI {

class Resource
{
public:
    Resource(uint8_t* data, size_t size) : data(data), size(size) {}
    ~Resource() { free(data); }

    int u16(size_t offset) { return data[offset] | (data[offset+1] << 8); }
    int16_t s16(size_t offset) { return data[offset] | (data[offset+1] << 8); }

    uint8_t* data;
    size_t size;
};

template<typename T> class ResourceManager
{
public:
    ResourceManager(const char* dirfile)
    {
        for(int n=0; n<256; n++) {
            res[n] = nullptr;
            resource_info[n] = 0xFFFFFF;
        }
        File f(dirfile);
        uint8_t buffer[3];
        int idx = 0;
        while (f.read(buffer, 3) == 3 && idx < 256) {
            resource_info[idx++] = (buffer[0] << 16) | (buffer[1] << 8) | (buffer[2] << 0);
            // printf("%s: %d: %x\n", dirfile, idx-1, resource_info[idx-1]);
        }
    }

    T* load(int index) {
        if (resource_info[index] >= 0xFFFFFF) return nullptr;
        if (res[index]) return res[index];
        char filename[16];
        sprintf(filename, "VOL.%d", resource_info[index] >> 20);
        File f(filename);
        if (!f.isOpen()) return nullptr;
        f.seek(resource_info[index] & 0x0FFFFF);
        uint8_t header[5];
        if (f.read(header, sizeof(header)) != 5)
            return nullptr;
        assert(header[0] == 0x12);
        assert(header[1] == 0x34);
        assert(header[2] == resource_info[index] >> 20);
        size_t size = header[3] | (header[4] << 8);
        uint8_t* data = (uint8_t*)malloc(size);
        if (f.read(data, size) != size) {
            free(data);
            return nullptr;
        }
        res[index] = new T(data, size);
        return res[index];
    }

    void unload(int index) {
        if (res[index]) {
            delete res[index];
            res[index] = nullptr;
        }
    }

    T* operator[](size_t idx) {
        return res[idx];
    }

private:
    T* res[256];
    uint32_t resource_info[256];
};

}