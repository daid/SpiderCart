#include <stdio.h>
#include <string.h>

#include "pins.h"
#include "memchip.h"
#include "mbc.h"
#include "command.h"
#include "fatfs/ff.h"


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

void start_mbc() {
    if (!core1_running) {
        multicore_launch_core1(core1_mbc_handler);
        sleep_ms(5);
        gpio_put(PIN_GB_RST, true);
        core1_running = true;
    }
}

void stop_mbc() {
    if (core1_running) {
        gpio_put(PIN_GB_RST, false);
        multicore_reset_core1();
        gpio_put(PIN_MEM_OE, true);
        gpio_put(PIN_MEM_WE, true);
        core1_running = false;
    }
}


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
            start_mbc();
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
    start_mbc();

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
        if (multicore_fifo_rvalid()) {
            auto cmd = sio_hw->fifo_rd;
            if (cmd & 0x100) {
                switch(cmd & 0xFF) {
                case COMMAND_LIST_DIR:
                    {
                        auto* ptr = ram_data;
                        FATFS fatfs;
                        memset(&fatfs, 0, sizeof(fatfs));
                        if (f_mount(&fatfs, "", 0) == FR_OK) {
                            DIR dir;
                            if (f_opendir(&dir, "/") == FR_OK) {
                                FILINFO fno;
                                while(f_readdir(&dir, &fno) == FR_OK && fno.fname[0]) {
                                    if (fno.fattrib & AM_DIR) {
                                        *ptr++ = 0x02;
                                        strcpy((char*)ptr, fno.fname);
                                        ptr += 31;
                                    } else {
                                        auto ext = strrchr(fno.fname, '.');
                                        if (strcasecmp(ext, ".gb") == 0 || strcasecmp(ext, ".gbc") == 0) {
                                            *ptr++ = 0x01;
                                            strcpy((char*)ptr, fno.fname);
                                            ptr += 31;
                                        }
                                    }
                                }
                                f_closedir(&dir);
                            }
                            f_unmount("");
                        }
                        *ptr = 0;
                        ram_data[15 * 0x2000 + 0x1FFF] = 0;
                    }
                    break;
                case COMMAND_LOAD_AND_RESET:
                    {
                        FATFS fatfs;
                        memset(&fatfs, 0, sizeof(fatfs));
                        if (f_mount(&fatfs, "", 0) == FR_OK) {
                            FIL fp;
                            if (f_open(&fp, (const char*)&ram_data[15 * 0x2000], FA_READ) == FR_OK) {
                                stop_mbc();
                                UINT br;
                                uint32_t addr = 0;
                                while(f_read(&fp, ram_data, 2048, &br) == FR_OK && br > 0) {
                                    //printf("%x %d\r\n", addr, br);
                                    write_memchip(addr, ram_data, br);
                                    read_memchip(addr, ram_data + br, br);
                                    if (memcmp(ram_data, ram_data + br, br) != 0) {
                                        printf("Fail?\r\n");
                                    }
                                    if (addr == 0) {
                                        for(int n=0; n<1024; n+=16) {
                                            printf("%08x  ", n);
                                            for(int m=0; m<16; m++) {
                                                printf("%02x ", ram_data[n + m]);
                                                if (m == 7) printf(" ");
                                            }
                                            printf("\r\n");
                                        }
                                    }
                                    addr += br;
                                }
                                start_mbc();
                            }
                            f_unmount("");
                        }
                    }
                    break;
                default:
                    ram_data[15 * 0x2000 + 0x1FFF] = 0;
                    break;
                }
            }
        }
    }
    return 0;
}
