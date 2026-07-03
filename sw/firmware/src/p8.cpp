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

static constexpr unsigned int RAM_DISPLAY_IDX = 15 * 0x2000; //0x1000 bytes
static constexpr unsigned int RAM_SFX_PLAY_IDX = 15 * 0x2000 + 0x1D00; //0x15 bytes: 1 byte flags, 5x4 bytes sound data
static constexpr unsigned int RAM_PAL_MAPPING_IDX = 15 * 0x2000 + 0x1E00; //0x10 bytes
static constexpr unsigned int RAM_BUTTON_SWAP_IDX = 15 * 0x2000 + 0x1E10; //1 byte
static constexpr unsigned int RAM_BUTTON_INPUT_IDX = 15 * 0x2000 + 0x1FF0; //1 byte

const uint8_t p8_gb_data[] = {
    #include "p8.gb.inc"
};

static const uint8_t p8_default_pal_lookup[16] = {
    3, 2, 2, 1,
    1, 2, 1, 0,
    1, 0, 0, 0,
    2, 1, 0, 0
};

static lua_State* p8_lua_state;
static bool p8_lua_60fps;

int p8lua_error() {
    co_error(1, "%s", lua_tostring(p8_lua_state, -1));
    lua_close(p8_lua_state);
    p8_lua_state = nullptr;
    return 1;
}

int p8_load(const char* filename)
{
    if (p8_lua_state) {
        lua_close(p8_lua_state);
        p8_lua_state = nullptr;
    }

    FATFS fatfs;
    memset(&fatfs, 0, sizeof(fatfs));
    if (f_mount(&fatfs, "", 0) != FR_OK) {
        return co_error(1, "mount failed");
    }
    FIL fp;
    if (f_open(&fp, filename, FA_READ) != FR_OK)
        return co_error(2, "read error");

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
        return co_error(3, "image error");;
    if (x != 160 || y != 205)
        return co_error(4, "image error");;
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

    p8_state.card_data = ram_data;

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
            return p8lua_error();
        }
    } else if (memcmp(ram_data + 0x4300, "\x00pxa", 4) == 0) { // new compression
        uint8_t mapping[256];
        for(int n=0; n<256; n++) mapping[n] = n;
        uint32_t decompressed_size = (ram_data[0x4304] << 8) | ram_data[0x4305];
        //uint32_t compressed_size = (ram_data[0x4306] << 8) | ram_data[0x4307];
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
            return p8lua_error();
        }
    } else {
        if (luaL_loadbufferx(p8_lua_state, (char*)ram_data + 0x4300, strlen((char*)ram_data + 0x4300), "=", nullptr) != LUA_OK) {
            return p8lua_error();
        }
    }

    if (lua_pcall(p8_lua_state, 0, 0, 0) != LUA_OK) {
        return p8lua_error();
    }

    lua_getglobal(p8_lua_state, "_init");
    if (lua_isfunction(p8_lua_state, -1)) {
        auto res = lua_pcall(p8_lua_state, 0, 0, 0);
        if (res) {
            return p8lua_error();
        }
    } else {
        lua_pop(p8_lua_state, 1);
    }

    lua_getglobal(p8_lua_state, "_update60");
    p8_lua_60fps = lua_isfunction(p8_lua_state, -1);
    lua_pop(p8_lua_state, 1);

    write_memchip(0, p8_gb_data, sizeof(p8_gb_data));
    memcpy(&ram_data[RAM_PAL_MAPPING_IDX], p8_default_pal_lookup, sizeof(p8_default_pal_lookup));
    ram_data[RAM_BUTTON_SWAP_IDX] = 0; // button swap

    return 0;
}

static int p8_update()
{
    if (!p8_lua_state)
        return 1;

    auto prev_button_mask = p8_state.button_mask;
    p8_state.button_mask = 0;
    if (ram_data[RAM_BUTTON_SWAP_IDX]) {
        if (ram_data[RAM_BUTTON_INPUT_IDX] & 0x01) p8_state.button_mask |= P8State::BTN_O;
        if (ram_data[RAM_BUTTON_INPUT_IDX] & 0x02) p8_state.button_mask |= P8State::BTN_X;
    } else {
        if (ram_data[RAM_BUTTON_INPUT_IDX] & 0x02) p8_state.button_mask |= P8State::BTN_O;
        if (ram_data[RAM_BUTTON_INPUT_IDX] & 0x01) p8_state.button_mask |= P8State::BTN_X;
    }
    if (ram_data[RAM_BUTTON_INPUT_IDX] & 0x20) p8_state.button_mask |= P8State::BTN_LEFT;
    if (ram_data[RAM_BUTTON_INPUT_IDX] & 0x10) p8_state.button_mask |= P8State::BTN_RIGHT;
    if (ram_data[RAM_BUTTON_INPUT_IDX] & 0x40) p8_state.button_mask |= P8State::BTN_UP;
    if (ram_data[RAM_BUTTON_INPUT_IDX] & 0x80) p8_state.button_mask |= P8State::BTN_DOWN;
    p8_state.button_pressed_mask = p8_state.button_mask & ~prev_button_mask;

    if (p8_lua_60fps)
        lua_getglobal(p8_lua_state, "_update60");
    else
        lua_getglobal(p8_lua_state, "_update");
    if (lua_isfunction(p8_lua_state, -1)) {
        auto res = lua_pcall(p8_lua_state, 0, 0, 0);
        if (res) {
            return p8lua_error();
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
            return p8lua_error();
        }
    } else {
        lua_pop(p8_lua_state, 1);
    }
    p8_render();
    return 0;
}

void p8_render()
{
    for(int ty=0; ty<16; ty++) {
        for(int tx=0; tx<16; tx++) {
            for(int y=0; y<8; y++) {
                uint8_t a = 0, b = 0;
                for(int x=0; x<8; x++) {
                    auto c = ram_data[RAM_PAL_MAPPING_IDX + p8_state.screen[tx*8+x + (ty*8+y)*128]];
                    if (c & 1) a |= 0x80 >> x;
                    if (c & 2) b |= 0x80 >> x;
                }
                ram_data[RAM_DISPLAY_IDX + tx * 16 + ty * 16 * 16 + y * 2] = a;
                ram_data[RAM_DISPLAY_IDX + tx * 16 + ty * 16 * 16 + y * 2 + 1] = b;
            }
        }
    }
}

//HZ = 131072 / (2048 - period)
#define FREQ_TO_GB_PERIOD(HZ) uint16_t(2048.0 - (131072.0 / (HZ)))

static const uint16_t pitch_to_period[64] = {
    FREQ_TO_GB_PERIOD(65.27),
    FREQ_TO_GB_PERIOD(68.97),
    FREQ_TO_GB_PERIOD(73.35),
    FREQ_TO_GB_PERIOD(77.72),
    FREQ_TO_GB_PERIOD(82.09),
    FREQ_TO_GB_PERIOD(87.14),
    FREQ_TO_GB_PERIOD(92.19),
    FREQ_TO_GB_PERIOD(97.9),
    FREQ_TO_GB_PERIOD(103.63),
    FREQ_TO_GB_PERIOD(109.68),
    FREQ_TO_GB_PERIOD(116.42),
    FREQ_TO_GB_PERIOD(122.81),
    FREQ_TO_GB_PERIOD(130.55),
    FREQ_TO_GB_PERIOD(138.28),
    FREQ_TO_GB_PERIOD(146.69),
    FREQ_TO_GB_PERIOD(155.45),
    FREQ_TO_GB_PERIOD(164.53),
    FREQ_TO_GB_PERIOD(174.29),
    FREQ_TO_GB_PERIOD(184.71),
    FREQ_TO_GB_PERIOD(195.82),
    FREQ_TO_GB_PERIOD(207.59),
    FREQ_TO_GB_PERIOD(219.71),
    FREQ_TO_GB_PERIOD(232.83),
    FREQ_TO_GB_PERIOD(245.95),
    FREQ_TO_GB_PERIOD(261-43),
    FREQ_TO_GB_PERIOD(276.9),
    FREQ_TO_GB_PERIOD(293.38),
    FREQ_TO_GB_PERIOD(310.89),
    FREQ_TO_GB_PERIOD(329.39),
    FREQ_TO_GB_PERIOD(348.91),
    FREQ_TO_GB_PERIOD(369.77),
    FREQ_TO_GB_PERIOD(391.97),
    FREQ_TO_GB_PERIOD(415.18),
    FREQ_TO_GB_PERIOD(439.75),
    FREQ_TO_GB_PERIOD(466),
    FREQ_TO_GB_PERIOD(491.91),
    FREQ_TO_GB_PERIOD(522.86),
    FREQ_TO_GB_PERIOD(553.81),
    FREQ_TO_GB_PERIOD(586.79),
    FREQ_TO_GB_PERIOD(621.76),
    FREQ_TO_GB_PERIOD(658.79),
    FREQ_TO_GB_PERIOD(697.81),
    FREQ_TO_GB_PERIOD(739.87),
    FREQ_TO_GB_PERIOD(783.94),
    FREQ_TO_GB_PERIOD(830.72),
    FREQ_TO_GB_PERIOD(879.85),
    FREQ_TO_GB_PERIOD(931.99),
    FREQ_TO_GB_PERIOD(983.82),
    FREQ_TO_GB_PERIOD(1045.72),
    FREQ_TO_GB_PERIOD(1107.61),
    FREQ_TO_GB_PERIOD(1173.56),
    FREQ_TO_GB_PERIOD(1243.56),
    FREQ_TO_GB_PERIOD(1317.58),
    FREQ_TO_GB_PERIOD(1395.62),
    FREQ_TO_GB_PERIOD(1479.74),
    FREQ_TO_GB_PERIOD(1567.88),
    FREQ_TO_GB_PERIOD(1661.43),
    FREQ_TO_GB_PERIOD(1759.67),
    FREQ_TO_GB_PERIOD(1863.97),
    FREQ_TO_GB_PERIOD(1967.59),
    FREQ_TO_GB_PERIOD(2091.41),
    FREQ_TO_GB_PERIOD(2215.22),
    FREQ_TO_GB_PERIOD(2347.12),
    FREQ_TO_GB_PERIOD(2487.07),
};

void p8_sfx()
{
    ram_data[RAM_SFX_PLAY_IDX] = 0;
    for(int n=0; n<4; n++) {
        auto& sfx = p8_state.sfx[n];
        if (!sfx.active) continue;
        auto data = &p8_state.card_data[0x3200 + sfx.sfx_nr * 68];
        while(sfx.speed_counter <= 0) {
            auto volume = (data[sfx.sample_idx * 2 + 1] >> 1) & 0x07;
            if (volume) {
                auto pitch = data[sfx.sample_idx * 2] & 0x3F;
                
                if ((ram_data[RAM_SFX_PLAY_IDX] & 1) == 0) {
                    ram_data[RAM_SFX_PLAY_IDX+1] = 0x00; //no volume sweep
                    ram_data[RAM_SFX_PLAY_IDX+2] = 0xBA; //50% duty, slightly more then 1 frame timing
                    ram_data[RAM_SFX_PLAY_IDX+3] = volume << 5; // no volume sweep
                    ram_data[RAM_SFX_PLAY_IDX+4] = pitch_to_period[pitch] & 0xFF;
                    ram_data[RAM_SFX_PLAY_IDX+5] = (pitch_to_period[pitch] >> 8) | 0xC0; // trigger+length enable
                    ram_data[RAM_SFX_PLAY_IDX] |= 1;
                } else if ((ram_data[RAM_SFX_PLAY_IDX] & 2) == 0) {
                    ram_data[RAM_SFX_PLAY_IDX+6] = 0xBA; //50% duty, slightly more then 1 frame timing
                    ram_data[RAM_SFX_PLAY_IDX+7] = volume << 5; // no volume sweep
                    ram_data[RAM_SFX_PLAY_IDX+8] = pitch_to_period[pitch] & 0xFF;
                    ram_data[RAM_SFX_PLAY_IDX+9] = (pitch_to_period[pitch] >> 8) | 0xC0; // trigger+length enable
                    ram_data[RAM_SFX_PLAY_IDX] |= 2;
                }
            }
            sfx.sample_idx += 1;
            if (sfx.sample_idx >= 32) {
                sfx.active = false;
                break;
            }
            if (data[65])
                sfx.speed_counter += data[65];
            else
                sfx.speed_counter += 1;
        }
        sfx.speed_counter -= 2;
    }
}

int p8_cycle30()
{
    //Called on DMG at 30FPS, so we can _update and _draw once and be done.
    p8_sfx();
    if (auto res = p8_update())
        return res;
    if (p8_lua_60fps) {
        if (auto res = p8_update())
            return res;
    }
    if (auto res = p8_draw())
        return res;
    p8_state.time += 1.0/30.0;
    return 0;
}

int p8_cycle60()
{
    static bool odd_frame = false;
    //Called on GBC at 60FPS
    if (!p8_lua_state)
        return 1;
    p8_sfx();
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
    p8_state.time += 1.0/60.0;
    return 0;
}