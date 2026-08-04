#include <stdint.h>

uint8_t execCommand(uint8_t);
uint8_t execQuickboot(uint8_t);
void clearScreen(void);
void printStr(uint16_t yx, const char* str);
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
    printStr(0, "Loading file list...");
    execCommand(0x01); //List files
    *((uint8_t*)0x4000) = 0x00; // Switch to bank 0
    printStr(0, "                    ");

    uint8_t* ptr = (uint8_t*)0xA000;
    uint16_t pos = 1;
    while(*ptr) {
        uint8_t bank = ptr[1];
        char* str_addr = *(char**)&ptr[2];
        *((volatile uint8_t*)0x4000) = bank; // Switch to bank containing filename
        printStr(pos, str_addr);
        *((volatile uint8_t*)0x4000) = 0x00; // Switch to bank 0
        ptr += 4;
        pos += 0x100;
    }
    pos = 0;
    printStr(pos, ">");
    while(1) {
        waitVBlank();
        updateJoypadState();
        if (JoypadPressed & PADF_DOWN) {
            if (((char*)0xA000)[(pos + 1) * 4]) {
                printStr(pos << 8, " ");
                pos += 1;
                printStr(pos << 8, ">");
            }
        }
        if (JoypadPressed & PADF_UP) {
            if (pos > 0) {
                printStr(pos << 8, " ");
                pos -= 1;
                printStr(pos << 8, ">");
            }
        }
        if (JoypadPressed & PADF_A) {
            uint8_t bank = *(char*)(0xA001 + pos * 4);
            char* str_addr = *(char**)(0xA002 + pos * 4);
            *((uint8_t*)0x4000) = bank; // Switch to bank with filename
            strcpy(tempBuffer, str_addr);
            *((uint8_t*)0x4000) = 0x0F; // Switch to bank 15
            strcpy((char*)0xA000, tempBuffer);
            printStr(0, "Loading ROM...");
            if (execCommand(0x10)) { //Load file and reset
                // Error
                clearScreen();
                printStr(0x000, "ERROR:");
                printStr(0x100, (const char*)0xBF00);
                do {
                    waitVBlank();
                    updateJoypadState();
                } while((JoypadPressed & (PADF_A | PADF_B)) == 0);
            }
            *((uint8_t*)0x4000) = 0x00; // Switch to bank 0
       }
        if (JoypadPressed & PADF_START) {
            uint8_t bank = *(char*)(0xA001 + pos * 4);
            char* str_addr = *(char**)(0xA002 + pos * 4);
            *((uint8_t*)0x4000) = bank; // Switch to bank with filename
            strcpy(tempBuffer, str_addr);
            *((uint8_t*)0x4000) = 0x0F; // Switch to bank 15
            strcpy((char*)0xA000, tempBuffer);
            execQuickboot(0x11); // Load quickboot
            *((uint8_t*)0x4000) = 0x00; // Switch to bank 0
       }
    }
}