#include <stdio.h>
#include <string.h>

#include "pins.h"
#include "memchip.h"
#include "mbc.h"
#include "mbc_prepare.h"
#include "command.h"
#include "romload.h"
#include "coprocessor.h"
#include "fatfs/ff.h"


#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "hardware/spi.h"
#include "hardware/pio.h"

#include "data_write.pio.h"
#include "data_read.pio.h"


extern uint8_t ram_data[16 * 0x2000];

uint8_t serial_receive_buffer[255];
uint8_t serial_receive_index;
uint8_t serial_receive_length;
enum class SerialReceiveState {
    StartChar,
    Length,
    Data
};
SerialReceiveState serial_receive_state = SerialReceiveState::StartChar;

uint32_t ram_write_idx = 0;
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
            //For USB loaded roms, never sav
            clear_sav_filename();
            prepare_mbc();
            start_mbc(true);
            printf(".");
        } else {
            printf("!");
        }
        break;
    case 0x02:
        if (core1_running) {
            stop_mbc();
            printf(".");
        } else {
            printf("!");
        }
        break;
    case 0x20:
        if (serial_receive_length == 4) {
            ram_write_idx = serial_receive_buffer[1] | (serial_receive_buffer[2] << 8) | (serial_receive_buffer[3] << 16);
            printf(".");
        } else {
            printf("?");
        }
        break;
    case 0x21:
        for(int n=1; n<serial_receive_length; n++) {
            ram_data[ram_write_idx] = serial_receive_buffer[n];
            ram_write_idx = (ram_write_idx + 1) % sizeof(ram_data);
        }
        printf(".");
        break;
    case 0x22:
        if (serial_receive_length == 4) {
            auto ram_read_idx = serial_receive_buffer[1] | (serial_receive_buffer[2] << 8) | (serial_receive_buffer[3] << 16);
            for(int n=0; n<255; n++)
                stdio_putchar_raw(ram_data[(ram_read_idx + n) % sizeof(ram_data)]);
        } else {
            printf("!");
        }
        break;
    default:
        printf("?");
        break;
    }
}

const uint8_t loader_gb_data[] = {
    #include "Loader.gb.inc"
};

int main() {
    hwInit();

    write_memchip(0, loader_gb_data, sizeof(loader_gb_data));
    start_mbc(true);

    stdio_init_all();
    while(1) {
        __wfe();
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

        processCoProcessor();
    }
    return 0;
}
