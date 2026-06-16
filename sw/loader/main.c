#include <stdint.h>

void printStr(uint16_t yx, char* str);
void updateJoypadState(void) __preserves_regs(d, e);
void waitVBlank(void) __preserves_regs(b, c, d, e, h, l);
extern uint8_t JoypadState;
extern uint8_t JoypadPressed;
#define PADF_DOWN   0x80
#define PADF_UP     0x40
#define PADF_LEFT   0x20
#define PADF_RIGHT  0x10
#define PADF_START  0x08
#define PADF_SELECT 0x04
#define PADF_B      0x02
#define PADF_A      0x01

void strcpy(char* dst, char* src)
{
    while(*src) {
        *dst++ = *src++;
    }
    *dst = 0;
}

char tempBuffer[32];

void main(void)
{
    *((uint8_t*)0x0000) = 0x0A; // enable SRAM
    *((uint8_t*)0x4000) = 0x0F; // Switch to bank 15
    *((uint8_t*)0xBFFF) = 0xFF; // Clear the "ready" flag
    *((uint8_t*)0x6000) = 0x01; // Command to read file list
    while (*((uint8_t*)0xBFFF) == 0xFF) { // Wait till ready
    }
    *((uint8_t*)0x4000) = 0x00; // Switch to bank 0

    char* ptr = (char*)0xA000;
    uint16_t pos = 1;
    while(*ptr) {
        printStr(pos, ptr + 1);
        ptr += 32;
        pos += 0x20;
    }
    pos = 0;
    printStr(pos, ">");
    while(1) {
        waitVBlank();
        updateJoypadState();
        if (JoypadPressed & PADF_DOWN) {
            if (((char*)0xA000)[(pos + 1) * 32]) {
                printStr(pos * 0x20, " ");
                pos += 1;
                printStr(pos * 0x20, ">");
            }
        }
        if (JoypadPressed & PADF_UP) {
            if (pos > 0) {
                printStr(pos * 0x20, " ");
                pos -= 1;
                printStr(pos * 0x20, ">");
            }
        }
        if (JoypadPressed & PADF_A) {
            strcpy(tempBuffer, (char*)0xA001 + pos * 0x20);
            *((uint8_t*)0x4000) = 0x0F; // Switch to bank 15
            strcpy((char*)0xA000, tempBuffer);
            *((uint8_t*)0xBFFF) = 0xFF; // Clear the "ready" flag
            *((uint8_t*)0x6000) = 0x10; // Command to load file&reset
            while (*((uint8_t*)0xBFFF) == 0xFF) { // Wait till ready
            }
            *((uint8_t*)0x4000) = 0x00; // Switch to bank 0
       }
    }
}