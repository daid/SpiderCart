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

void processSerialMessage()
{
    switch(serial_receive_buffer[0]) {
    case 0x10:
        if (serial_receive_length > 4 && !core1_running) {
            write_memchip(serial_receive_buffer[1] | (serial_receive_buffer[2] << 8) | (serial_receive_buffer[3] << 16), serial_receive_buffer + 4, serial_receive_length - 4);
            printf(".");
        } else {
            printf("!");
        }
        break;
    case 0x11:
        if (serial_receive_length == 4 && !core1_running) {
            read_memchip(serial_receive_buffer[1] | (serial_receive_buffer[2] << 8) | (serial_receive_buffer[3] << 16), serial_receive_buffer, 255);
            for(int n=0; n<255; n++)
                stdio_putchar_raw(serial_receive_buffer[n]);
        } else {
            printf("!");
        }
        break;
    case 0x01:
        if (!core1_running) {
            multicore_launch_core1(core1_mbc_handler);
            sleep_ms(5);
            gpio_put(PIN_GB_RST, true);
            printf(".");
            core1_running = true;
        } else {
            printf("!");
        }
        break;
    case 0x02:
        if (core1_running) {
            gpio_put(PIN_GB_RST, false);
            multicore_reset_core1();
            gpio_put(PIN_MEM_OE, true);
            gpio_put(PIN_MEM_WE, true);
            core1_running = false;
            printf(".");
        } else {
            printf("!");
        }
        break;
    default:
        printf("?");
        break;
    }
}

int main() {
    hwInit();

    stdio_init_all();
    while(1) {
        int c = stdio_getchar_timeout_us(0);
        if (c >= 0) {
            switch(serial_receive_state) {
            case SerialReceiveState::StartChar:
                if (c == 0x5A) serial_receive_state = SerialReceiveState::Length;
                break;
            case SerialReceiveState::Length:
                serial_receive_index = 0;
                serial_receive_length = c;
                if (serial_receive_length > 0)
                    serial_receive_state = SerialReceiveState::Data;
                else
                    serial_receive_state = SerialReceiveState::StartChar;
                break;
            case SerialReceiveState::Data:
                serial_receive_buffer[serial_receive_index++] = c;
                if (serial_receive_index == serial_receive_length) {
                    processSerialMessage();
                    serial_receive_state = SerialReceiveState::StartChar;
                }
                break;
            }
        }
        /*
        if (c == 'S') {
            printf("\nI2C Bus Scan\n");
            printf("   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");
            for (int addr = 0; addr < (1 << 7); ++addr) {
                if (addr % 16 == 0) {
                    printf("%02x ", addr);
                }

                // Perform a 1-byte dummy read from the probe address. If a slave
                // acknowledges this address, the function returns the number of bytes
                // transferred. If the address byte is ignored, the function returns
                // -1.

                // Skip over any reserved addresses.
                int ret;
                uint8_t rxdata;
                if (reserved_addr(addr))
                    ret = PICO_ERROR_GENERIC;
                else
                    ret = i2c_read_blocking(i2c1, addr, &rxdata, 1, false);

                printf(ret < 0 ? "." : "@");
                printf(addr % 16 == 15 ? "\n" : "  ");
            }
            printf("Done.\n");
        }
        if (c == 'M') {
            for(uint32_t addr = 0; addr < 0x8000; addr+=4) {
                write_memchip(addr, (uint8_t*)&addr, 4);
            }
            for(uint32_t addr = 0; addr < 0x8000; addr+=4) {
                uint32_t tmp = 0;
                read_memchip(addr, (uint8_t*)&tmp, 4);
                if (tmp != addr)
                    printf("Fail: %08x != %08x\n", addr, tmp);
            }
            printf("MEMCHECK DONE\n");
        }
        if (c == 'A') {
            uint8_t data[6];
            data[0] = 0x20; // CTRL_REG1
            data[1] = 0x57;
            i2c_write_blocking(i2c1, 0x19, data, 2, false);
            for(int n=0; n<32; n++) {
                data[0] = 0x28 | 0x80;
                i2c_write_blocking(i2c1, 0x19, data, 1, true);
                i2c_read_blocking(i2c1, 0x19, data, 6, false);
                printf("%x ", data[0] | (data[1] << 8));
                printf("%x ", data[2] | (data[3] << 8));
                printf("%x\n", data[4] | (data[5] << 8));
                sleep_ms(10);
            }
        }
        if (c == 'C') {
            uint8_t data[16];
            data[0] = 0x00;
            i2c_write_blocking(i2c1, 0x51, data, 1, true);
            i2c_read_blocking(i2c1, 0x51, data, 16, false);
            for(int n=0; n<16; n++) printf("%02x: %02x\n", n, data[n]);
        }
        */
    }
    return 0;
}
