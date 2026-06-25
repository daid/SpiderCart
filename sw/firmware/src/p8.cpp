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


static lua_State* p8_lua_state;
static bool p8_lua_60fps;

void p8_error(const char* fmt, ...)
{
    va_list arg_ptr;

    va_start(arg_ptr, fmt);
    vsnprintf((char*)&ram_data[15 * 0x2000 + 0x1F00], 0xF0, fmt, arg_ptr);
    printf("%s\n", (char*)&ram_data[15 * 0x2000 + 0x1F00]);
    va_end(arg_ptr);
    ram_data[15 * 0x2000 + 0x1FF1] = 1;
}

int p8_load(const char* filename)
{
    ram_data[15 * 0x2000 + 0x1FF1] = 0;
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
        uint32_t decompressed_size = (ram_data[0x4304] << 8) | ram_data[0x4305];
        auto write_ptr = ram_data + 0x11000;
        read_ptr = ram_data + 0x4308;
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
        if (luaL_loadbufferx(p8_lua_state, (char*)ram_data + 0x11000, decompressed_size, "=", nullptr) != LUA_OK) {
            p8_error("%s", lua_tostring(p8_lua_state, -1));
            lua_close(p8_lua_state);
            p8_lua_state = nullptr;
            return 3;
        }
    } else if (memcmp(ram_data + 0x4300, "\x00pxa", 4) == 0) { // new compression
        uint8_t mapping[256];
        for(int n=0; n<256; n++) mapping[n] = n;
        uint32_t decompressed_size = (ram_data[0x4304] << 8) | ram_data[0x4305];
        uint32_t compressed_size = (ram_data[0x4306] << 8) | ram_data[0x4307];
        auto read_ptr = &ram_data[0x4308];
        auto bit_mask = 0x01;
        auto write_ptr = ram_data + 0x11000;
        auto get_bit = [&]() {
            bool res = (*read_ptr) & bit_mask;
            bit_mask <<= 1;
            if (bit_mask == 0x100) {
                read_ptr++;
                bit_mask = 0x01;
            }
            return res;
        };
        auto get_bits = [&](int cnt) {
            int res = 0;
            for(int n=0; n<cnt; n++) {
                if (get_bit())
                    res |= 1 << n;
            }
            return res;
        };
        while(write_ptr < ram_data + 0x11000 + decompressed_size) {
            if (get_bit()) {
                int unary = 0;
                while(get_bit()) unary += 1;
                int unary_mask = ((1 << unary) - 1);
                int index = get_bits(4 + unary) + (unary_mask << 4);
                auto value = mapping[index];
                for(int n=index;n>0;n--) mapping[n] = mapping[n-1];
                mapping[0] = value;
                *write_ptr++ = value;
            } else {
                int offset_bits = 15;
                if (get_bit()) {
                    offset_bits = get_bit() ? 5 : 10;
                }
                int offset = get_bits(offset_bits) + 1;
                if (offset == 1 && offset_bits == 10) {
                    while(auto value = get_bits(8)) {
                        *write_ptr++ = value;
                    }
                } else {
                    int length = 3;
                    while(true) {
                        auto part = get_bits(3);
                        length += part;
                        if (part != 7)
                            break;
                    }
                    while(length) {
                        *write_ptr = write_ptr[-offset];
                        write_ptr++;
                        length -= 1;
                    }
                }
            }
        }
        if (luaL_loadbufferx(p8_lua_state, (char*)ram_data + 0x11000, decompressed_size, "=", nullptr) != LUA_OK) {
            p8_error("%s", lua_tostring(p8_lua_state, -1));
            lua_close(p8_lua_state);
            p8_lua_state = nullptr;
            return 3;
        }
    } else {
        if (luaL_loadbufferx(p8_lua_state, (char*)ram_data + 0x4300, strlen((char*)ram_data + 0x4300), "=", nullptr) != LUA_OK) {
            p8_error("%s", lua_tostring(p8_lua_state, -1));
            lua_close(p8_lua_state);
            p8_lua_state = nullptr;
            return 3;
        }
    }

    if (lua_pcall(p8_lua_state, 0, 0, 0) != LUA_OK) {
        p8_error("%s", lua_tostring(p8_lua_state, -1));
        lua_close(p8_lua_state);
        p8_lua_state = nullptr;
        return 4;
    }

    lua_getglobal(p8_lua_state, "_init");
    if (lua_isfunction(p8_lua_state, -1)) {
        auto res = lua_pcall(p8_lua_state, 0, 0, 0);
        if (res) {
            p8_error("%d: _init: %s", res, lua_tostring(p8_lua_state, -1));
            lua_close(p8_lua_state);
            p8_lua_state = nullptr;
            return 5;
        }
    } else {
        lua_pop(p8_lua_state, 1);
    }

    lua_getglobal(p8_lua_state, "_update60");
    p8_lua_60fps = lua_isfunction(p8_lua_state, -1);
    lua_pop(p8_lua_state, 1);

    return 0;
}

uint8_t p8_pal_lookup[16] = {
    3, 2, 2, 1,
    1, 2, 1, 0,
    1, 0, 0, 0,
    2, 1, 0, 0
};

static int p8_update()
{
    if (!p8_lua_state)
        return 1;

    auto prev_button_mask = p8lua_button_mask;
    p8lua_button_mask = 0;
    if (ram_data[15 * 0x2000 + 0x1FF0] & 0x02) p8lua_button_mask |= P8LUA_BTN_O;
    if (ram_data[15 * 0x2000 + 0x1FF0] & 0x01) p8lua_button_mask |= P8LUA_BTN_X;
    if (ram_data[15 * 0x2000 + 0x1FF0] & 0x20) p8lua_button_mask |= P8LUA_BTN_LEFT;
    if (ram_data[15 * 0x2000 + 0x1FF0] & 0x10) p8lua_button_mask |= P8LUA_BTN_RIGHT;
    if (ram_data[15 * 0x2000 + 0x1FF0] & 0x40) p8lua_button_mask |= P8LUA_BTN_UP;
    if (ram_data[15 * 0x2000 + 0x1FF0] & 0x80) p8lua_button_mask |= P8LUA_BTN_DOWN;
    p8lua_button_pressed_mask = p8lua_button_mask & ~prev_button_mask;

    if (p8_lua_60fps)
        lua_getglobal(p8_lua_state, "_update60");
    else
        lua_getglobal(p8_lua_state, "_update");
    if (lua_isfunction(p8_lua_state, -1)) {
        auto res = lua_pcall(p8_lua_state, 0, 0, 0);
        if (res) {
            p8_error("%d: _update: %s", res, lua_tostring(p8_lua_state, -1));
            lua_close(p8_lua_state);
            p8_lua_state = nullptr;
            return 2;
        }
    } else {
        lua_pop(p8_lua_state, 1);
    }
    return 0;
}

int p8_draw()
{
    if (!p8_lua_state)
        return 1;

    lua_getglobal(p8_lua_state, "_draw");
    if (lua_isfunction(p8_lua_state, -1)) {
        auto res = lua_pcall(p8_lua_state, 0, 0, 0);
        if (res) {
            p8_error("%d: _draw: %s", res, lua_tostring(p8_lua_state, -1));
            lua_close(p8_lua_state);
            p8_lua_state = nullptr;
            return 2;
        }
    } else {
        lua_pop(p8_lua_state, 1);
    }

    for(int ty=0; ty<16; ty++) {
        for(int tx=0; tx<16; tx++) {
            for(int y=0; y<8; y++) {
                uint8_t a = 0, b = 0;
                for(int x=0; x<8; x++) {
                    auto c = p8_pal_lookup[p8lua_screen[tx*8+x + (ty*8+y)*128]];
                    if (c & 1) a |= 0x80 >> x;
                    if (c & 2) b |= 0x80 >> x;
                }
                ram_data[15 * 0x2000 + tx * 16 + ty * 16 * 16 + y * 2] = a;
                ram_data[15 * 0x2000 + tx * 16 + ty * 16 * 16 + y * 2 + 1] = b;
            }
        }
    }
    return 0;
}

int p8_cycle30()
{
    //Called on DMG at 30FPS, so we can _update and _draw once and be done.
    if (auto res = p8_update())
        return res;
    if (p8_lua_60fps) {
        if (auto res = p8_update())
            return res;
    }
    if (auto res = p8_draw())
        return res;
    p8lua_time += 1.0/30.0;
    return 0;
}

int p8_cycle60()
{
    static bool odd_frame = false;
    //Called on GBC at 60FPS
    if (!p8_lua_state)
        return 1;
    if (!odd_frame) {
        if (auto res = p8_update())
            return res;
        if (auto res = p8_draw())
            return res;
    } else if (p8_lua_60fps) {
        if (auto res = p8_update())
            return res;
        if (auto res = p8_draw())
            return res;
    }

    odd_frame = !odd_frame;
    p8lua_time += 1.0/60.0;
    return 0;
}