#include <stdio.h>
#include <string.h>

#include "pins.h"
#include "memchip.h"
#include "mbc.h"


#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "hardware/spi.h"
#include "hardware/pio.h"

#include "data_write.pio.h"
#include "data_read.pio.h"


uint8_t ram_data[16 * 0x2000];

uint8_t serial_receive_buffer[255];
uint8_t serial_receive_index;
uint8_t serial_receive_length;
enum class SerialReceiveState {
    StartChar,
    Length,
    Data
};
SerialReceiveState serial_receive_state = SerialReceiveState::StartChar;
bool core1_running = false;

uint32_t temp_buffer[1024];

bool testMemPattern(uint32_t pattern)
{
    for(unsigned int n=0; n<sizeof(temp_buffer)/sizeof(uint32_t); n++) {
        temp_buffer[n] = pattern;
    }
    for(int addr=0; addr<MEMCHIP_SIZE; addr+=sizeof(temp_buffer)) {
        write_memchip(addr, (uint8_t*)temp_buffer, sizeof(temp_buffer));
    }
    for(int addr=0; addr<MEMCHIP_SIZE; addr+=sizeof(temp_buffer)) {
        read_memchip(addr, (uint8_t*)temp_buffer, sizeof(temp_buffer));
        for(unsigned int n=0; n<sizeof(temp_buffer)/sizeof(uint32_t); n++) {
            if (temp_buffer[n] != pattern)
                return false;
        }
    }
    return true;
}

bool testMemAddr()
{
    for(int addr=0; addr<MEMCHIP_SIZE; addr+=sizeof(temp_buffer)) {
        for(unsigned int n=0; n<sizeof(temp_buffer)/sizeof(uint32_t); n++) {
            temp_buffer[n] = addr + n;
        }
        write_memchip(addr, (uint8_t*)temp_buffer, sizeof(temp_buffer));
    }
    for(int addr=0; addr<MEMCHIP_SIZE; addr+=sizeof(temp_buffer)) {
        read_memchip(addr, (uint8_t*)temp_buffer, sizeof(temp_buffer));
        for(unsigned int n=0; n<sizeof(temp_buffer)/sizeof(uint32_t); n++) {
            if (temp_buffer[n] != addr + n)
                return false;
        }
    }
    return true;
}

bool testAccel()
{
    //Test LIS3DH
    uint8_t data[6];
    data[0] = 0x20; // CTRL_REG1
    data[1] = 0x57;
    if (i2c_write_timeout_us(i2c1, 0x19, data, 2, false, 1000000) != 2)
        return false;

    data[0] = 0x28 | 0x80;
    if (i2c_write_timeout_us(i2c1, 0x19, data, 1, true, 1000000) != 1)
        return false;
    if (i2c_read_timeout_us(i2c1, 0x19, data, 6, false, 1000000) != 6)
        return false;
    return true;
}

bool testRTC()
{
    //Test PCF8563TS
    uint8_t data[16];

    //Set seconds/minutes to 0
    data[0] = 0x02;
    data[1] = 0x00;
    data[2] = 0x00;
    if (i2c_write_timeout_us(i2c1, 0x51, data, 3, false, 1000000) != 3)
        return false;
    
    //Read seconds/minutes
    data[0] = 0x02;
    if (i2c_write_timeout_us(i2c1, 0x51, data, 1, true, 1000000) != 1)
        return false;
    if (i2c_read_timeout_us(i2c1, 0x51, data, 2, false, 1000000) != 2)
        return false;
    if (data[0] != 0) return false;
    if (data[1] != 0) return false;
    sleep_ms(3000);

    //Read seconds/minutes
    data[0] = 0x02;
    if (i2c_write_timeout_us(i2c1, 0x51, data, 1, true, 1000000) != 1)
        return false;
    if (i2c_read_timeout_us(i2c1, 0x51, data, 2, false, 1000000) != 2)
        return false;
    if (data[0] != 3) return false;
    if (data[1] != 0) return false;

    return true;
}

uint8_t cardCommand(uint8_t cmd, uint32_t arg) {
    uint8_t buffer[6];

    gpio_put(PIN_SD_CS, false);

    auto timeout = make_timeout_time_ms(300);
    // Wait not busy
    while(timeout > get_absolute_time()) {
        spi_read_blocking(spi1, 0xFF, buffer, 1);
        if (buffer[0] == 0xFF) break;
    }
    if (timeout <= get_absolute_time()) {
        gpio_put(PIN_SD_CS, true);
        return 0xFF;
    }

    buffer[0] = cmd | 0x40;
    buffer[1] = arg >> 24;
    buffer[2] = arg >> 16;
    buffer[3] = arg >> 8;
    buffer[4] = arg >> 0;
    uint8_t crc = 0xFF;
    if (cmd == 0x00) crc = 0X95;  // correct crc for CMD0 with arg 0
    if (cmd == 0x08) crc = 0X87;  // correct crc for CMD8 with arg 0X1AA
    buffer[5] = crc;
    spi_write_blocking(spi1, buffer, 6);
    for(int n=0; n<255; n++) {
        spi_read_blocking(spi1, 0xFF, buffer, 1);
        if (!(buffer[0] & 0x80)) break;
    }
    return buffer[0];
}

bool testSDCard()
{
    // Just check basic initialization.

    // must supply min of 74 clock cycles with CS high.
    for (uint8_t i = 0; i < 10; i++) spi_write_blocking(spi1, (const uint8_t*)"\xFF", 1);

    uint8_t status;
    int timeout = 0;
    uint8_t buffer[4];
    // command to go idle in SPI mode
    while ((status = cardCommand(0x00, 0)) != 0x01) {
        timeout ++;
        if (timeout > 32) break;
    }
    gpio_put(PIN_SD_CS, true); sleep_ms(1);

    if (status != 0x01) { return false; }
    status = cardCommand(0x08, 0x1AA);
    if (status != 0x01) { gpio_put(PIN_SD_CS, true); return false; }
    spi_read_blocking(spi1, 0xFF, buffer, 4);
    gpio_put(PIN_SD_CS, true); 
    if (buffer[2] != 0x01) return false;
    if (buffer[3] != 0xAA) return false;
    return true;
}

#define TEST(msg, func) do { printf(" %16s: ", msg); if (func) { printf("PASS\n"); } else { success = false; printf("FAIL\n"); } } while(0)

int main() {
    hwInit();

    stdio_init_all();
    while(!stdio_usb_connected()) {
        sleep_ms(200);
        gpio_put(PIN_RUMBLE, true);
        sleep_ms(200);
        gpio_put(PIN_RUMBLE, false);
    }
    while(true) {
        sleep_ms(100);
        printf("Starting tests...\n");
        bool success = true;
        printf("Memory test:\n");
        TEST("All zero", testMemPattern(0));
        TEST("All one", testMemPattern(~0));
        TEST("Alternating 01", testMemPattern(0x55555555));
        TEST("Alternating 10", testMemPattern(0xAAAAAAAA));
        TEST("Address", testMemAddr());
        printf("Chip tests:\n");
        TEST("Accel", testAccel());
        TEST("RTC", testRTC());
        TEST("SDCard", testSDCard());

        printf("Tests finished: %s\n", success ? "PASS" : "FAIL");
        while(1) {
            if (stdio_getchar_timeout_us(100000) > 0)
                break;
        }
    }
    return 0;
}
