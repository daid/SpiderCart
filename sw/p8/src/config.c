#include <stdint.h>


uint8_t execCommand(uint8_t);
void clearScreen(void);
void setBGPalette(void);
void printStr(uint16_t yx, char* str);
void setTileGBC(uint16_t yx, uint16_t tile_nr_attr);
void updateJoypadState(void) __preserves_regs(d, e);
void waitVBlank(void) __preserves_regs(b, c, d, e, h, l);
extern uint8_t JoypadState;
extern uint8_t JoypadPressed;
extern uint16_t BGPalette[32];
extern const uint16_t picoColors[16];
#define PADF_DOWN   0x80
#define PADF_UP     0x40
#define PADF_LEFT   0x20
#define PADF_RIGHT  0x10
#define PADF_START  0x08
#define PADF_SELECT 0x04
#define PADF_B      0x02
#define PADF_A      0x01

#define P8_PAL_MAP_ENTRY(n) (*((uint8_t*)0xBE00 + (n)))
#define P8_BUTTON_SWAP() (*((uint8_t*)0xBE10))

void colormapMenu(void) {
    uint8_t cursor = 1;
    clearScreen();
    while(1) {
        for(uint8_t n=0; n<16; n++) {
            setTileGBC( ((n+1) << 8) | 1, (0x7C04) | ((n & 0x03) << 8) | (n >> 2));
            printStr( ((n+1) << 8) | 2, "->");
            setTileGBC( ((n+1) << 8) | 4, 0x7C01 | (P8_PAL_MAP_ENTRY(n) << 8));
        }

        printStr((cursor << 8) | 0, ">");
        while(1) {
            waitVBlank();
            updateJoypadState();
            if (JoypadPressed & PADF_START)
                return;
            if (JoypadPressed & PADF_UP) {
                printStr((cursor << 8) | 0, " ");
                cursor -= 1;
                if (cursor == 0) cursor = 16;
                printStr((cursor << 8) | 0, ">");
            }
            if (JoypadPressed & PADF_DOWN) {
                printStr((cursor << 8) | 0, " ");
                cursor += 1;
                if (cursor == 17) cursor = 1;
                printStr((cursor << 8) | 0, ">");
            }
            if (JoypadPressed & PADF_LEFT) {
                P8_PAL_MAP_ENTRY(cursor - 1) = (P8_PAL_MAP_ENTRY(cursor - 1) + 3) & 3;
                break;
            }
            if (JoypadPressed & PADF_RIGHT) {
                P8_PAL_MAP_ENTRY(cursor - 1) = (P8_PAL_MAP_ENTRY(cursor - 1) + 1) & 3;
                break;
            }
        }
    }
}

void paletteMenu(void)
{
    uint8_t cursor = 1;
    uint8_t n = 0;
    clearScreen();
    while(1) {
        for(uint8_t n=0; n<4; n++) {
            setTileGBC( ((n+1) << 8) | 1, (0x7C01) | ((n & 0x03) << 8));
        }

        printStr((cursor << 8) | 0, ">");
        while(1) {
            waitVBlank();
            updateJoypadState();
            if (JoypadPressed & PADF_START)
                return;
            if (JoypadPressed & PADF_UP) {
                printStr((cursor << 8) | 0, " ");
                cursor -= 1;
                if (cursor == 0) cursor = 16;
                printStr((cursor << 8) | 0, ">");
            }
            if (JoypadPressed & PADF_DOWN) {
                printStr((cursor << 8) | 0, " ");
                cursor += 1;
                if (cursor == 5) cursor = 1;
                printStr((cursor << 8) | 0, ">");
            }
            if (JoypadPressed & PADF_LEFT) {
                n = (n + 15) & 0x0F;
                BGPalette[cursor + 3] = picoColors[n];
                setBGPalette();
                break;
            }
            if (JoypadPressed & PADF_RIGHT) {
                n = (n + 1) & 0x0F;
                BGPalette[cursor + 3] = picoColors[n];
                setBGPalette();
                break;
            }
        }
    }
}

void configMenu(void)
{
    uint8_t cursor = 1;
    while(1) {
        clearScreen();
        printStr(0, "PICO-8 CONFIG");
        printStr((1 << 8) | 1, "Colormap");
        printStr((2 << 8) | 1, "Palette");
        printStr((3 << 8) | 1, P8_BUTTON_SWAP() ? "[B] = [X] [A] = [O]" : "[B] = [O] [A] = [X]");

        printStr((cursor << 8) | 0, ">");
        while(1) {
            waitVBlank();
            updateJoypadState();
            if (JoypadPressed & PADF_START)
                return;
            if (JoypadPressed & PADF_UP) {
                printStr((cursor << 8) | 0, " ");
                cursor -= 1;
                if (cursor == 0) cursor = 5;
                printStr((cursor << 8) | 0, ">");
            }
            if (JoypadPressed & PADF_DOWN) {
                printStr((cursor << 8) | 0, " ");
                cursor += 1;
                if (cursor == 6) cursor = 1;
                printStr((cursor << 8) | 0, ">");
            }
            if (JoypadPressed & (PADF_A | PADF_B)) {
                if (cursor == 1) colormapMenu();
                if (cursor == 2) paletteMenu();
                if (cursor == 3) P8_BUTTON_SWAP() ^= 0x01;
                break;
            }
        }
    }
}