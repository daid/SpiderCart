#include "p8.h"
#include "p8lua.h"
#include "fatfs/ff.h"
#include "memchip.h"
#include "coprocessor.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_STDIO
#define STBI_NO_GIF
#define STBI_NO_HDR
#define STBI_NO_LINEAR
#define STBI_ONLY_PNG
#define STBI_NO_THREAD_LOCALS
#include "stb_image.h"

#include <cstring>


const uint8_t p8_gb_data[] = {
    #include "p8.gb.inc"
};


lua_State* p8_lua_state;


int load_p8(const char* filename)
{
    if (p8_lua_state) {
        lua_close(p8_lua_state);
        p8_lua_state = nullptr;
    }

    write_memchip(0, p8_gb_data, sizeof(p8_gb_data));

    FATFS fatfs;
    memset(&fatfs, 0, sizeof(fatfs));
    if (f_mount(&fatfs, "", 0) != FR_OK)
        return 1;
    FIL fp;
    if (f_open(&fp, filename, FA_READ) != FR_OK)
        return 2;

    stbi_io_callbacks callbacks = {
        [](void *user, char *data,int size) -> int {
            // fill 'data' with 'size' bytes.  return number of bytes actually read
            unsigned int res = 0;
            if (f_read((FIL*)user, data, size, &res) != FR_OK)
                return 0;
            return res;
        },
        [](void *user, int n) {
            // skip the next 'n' bytes, or 'unget' the last -n bytes if negative
            f_lseek((FIL*)user, f_tell((FIL*)user) + n);
        },
        [](void *user) -> int {
            // returns nonzero if we are at end of file/data
            return f_eof((FIL*)user);
        }
    };
    int x = 0, y = 0;
    auto pixels = stbi_load_from_callbacks(&callbacks, &fp, &x, &y, nullptr, 4);
    if (!pixels)
        return 1;
    if (x != 160 || y != 205)
        return 2;
    auto read_ptr = pixels;
    auto write_ptr = ram_data;
    for(y=0; y<205; y++) {
        for(x=0; x<160; x++) {
            auto value = ((*read_ptr++) & 0x03) << 4;
            value |= ((*read_ptr++) & 0x03) << 2;
            value |= ((*read_ptr++) & 0x03) << 0;
            value |= ((*read_ptr++) & 0x03) << 6;
            *write_ptr++ = value;
        }
    }
    stbi_image_free(pixels);
    f_close(&fp);
    f_unmount("");

    p8lua_card_data = ram_data;

    p8_lua_state = luaL_newstate();
    setupP8LuaEnv(p8_lua_state);
    if (memcmp(ram_data + 0x4300, ":c:", 4) == 0) { // old compression
        uint32_t decompressed_size = (ram_data[0x4304] << 8) | ram_data[0x3405];
        decompressed_size += 0x100;
        auto write_ptr = ram_data + 0x11000;
        read_ptr = ram_data + 0x4308;
        printf("decompressed_size: %d %02x %02x\n", decompressed_size, ram_data[0x4304], ram_data[0x4305]);
        while(write_ptr < ram_data + 0x11000 + decompressed_size) {
            if (*read_ptr == 0x00) {
                read_ptr++;
                *write_ptr++ = *read_ptr++;
            } else if (*read_ptr < 0x3C) {
                *write_ptr++ = "\n 0123456789abcdefghijklmnopqrstuvwxyz!#%(){}[]<>+=/*:;.,~_"[(*read_ptr++) - 1];
            } else {
                auto offset = (read_ptr[0] - 0x3c) * 16 + (read_ptr[1] & 0xf);
                auto length = (read_ptr[1] >> 4) + 2;
                read_ptr += 2;
                while(length) {
                    *write_ptr = write_ptr[-offset];
                    write_ptr++;
                    length -= 1;
                }
            }
        }
        decompressed_size = write_ptr - (ram_data + 0x11000);
        FILE* f = fopen("p8.lua", "wb");
        fwrite(ram_data + 0x11000, decompressed_size, 1, f);
        fclose(f);
        if (luaL_loadbufferx(p8_lua_state, (char*)ram_data + 0x11000, strlen((char*)ram_data + 0x11000), "=", nullptr) != LUA_OK) {
            printf("%s\n", lua_tostring(p8_lua_state, -1));
            lua_pop(p8_lua_state, 1);
            return 3;
        }
        if (lua_pcall(p8_lua_state, 0, 0, 0) != LUA_OK) {
            printf("%s\n", lua_tostring(p8_lua_state, -1));
            lua_pop(p8_lua_state, 1);
            return 4;
        }
    } else if (memcmp(ram_data + 0x4300, "\x00pxa", 4) == 0) { // new compression
        //TODO
        //luaL_loadbufferx(p8_lua_state, (char*)ram_data + 0x11000, strlen((char*)ram_data + 0x11000), "=", nullptr);
        return 100;
    } else {
        if (luaL_loadbufferx(p8_lua_state, (char*)ram_data + 0x4300, strlen((char*)ram_data + 0x4300), "=", nullptr) != LUA_OK) {
            printf("%s\n", lua_tostring(p8_lua_state, -1));
            lua_pop(p8_lua_state, 1);
            return 3;
        }
        if (lua_pcall(p8_lua_state, 0, 0, 0) != LUA_OK) {
            printf("%s\n", lua_tostring(p8_lua_state, -1));
            lua_pop(p8_lua_state, 1);
            return 4;
        }
    }

    return 0;
}
