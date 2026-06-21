#include <stdio.h>
#include <string.h>

#include "pins.h"
#include "memchip.h"
#include "mbc.h"
#include "command.h"
#include "romload.h"
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

static MBC_Type mbc_type = MBC_Type::Spider;
static uint32_t mbc_flags = 0;
static uint32_t mbc_rom_bank_mask = 0xFF;
static uint32_t mbc_ram_bank_mask = 0x0F;
static void core1_mbc_handler()
{
    core1_start_mbc(mbc_type, mbc_flags, mbc_rom_bank_mask, mbc_ram_bank_mask);
}

void start_mbc(bool reset) {
    if (!core1_running) {
        if (reset)
            gpio_put(PIN_GB_RST, false);
        multicore_launch_core1(core1_mbc_handler);
        sleep_ms(5);
        if (reset)
            gpio_put(PIN_GB_RST, true);
        core1_running = true;
    }
}

void stop_mbc() {
    if (core1_running) {
        multicore_reset_core1();
        gpio_put_all(PIN_MASK_MEM_OE | PIN_MASK_MEM_WE);
        data_write_set_in(pio0, 0);
        core1_running = false;
    }
}

void prepare_mbc()
{
    if (core1_running) return;
    uint8_t header_info[0x100];
    read_memchip(0x100, header_info, 0x100);
    switch(header_info[0x48]) {
    case 0x00: mbc_rom_bank_mask = 0x000; break; //32 KiB	2 (no banking)
    case 0x01: mbc_rom_bank_mask = 0x003; break; //64 KiB	4
    case 0x02: mbc_rom_bank_mask = 0x007; break; //128 KiB	8
    case 0x03: mbc_rom_bank_mask = 0x00F; break; //256 KiB	16
    case 0x04: mbc_rom_bank_mask = 0x01F; break; //512 KiB	32
    case 0x05: mbc_rom_bank_mask = 0x03F; break; //1 MiB	64
    case 0x06: mbc_rom_bank_mask = 0x07F; break; //2 MiB	128
    case 0x07: mbc_rom_bank_mask = 0x0FF; break; //4 MiB	256
    case 0x08: mbc_rom_bank_mask = 0x1FF; break; //8 MiB	512
    default: mbc_rom_bank_mask = 0x1FF; break;
    }
    switch(header_info[0x49]) {
    case 0x00: mbc_ram_bank_mask = 0x00; break;	// 0	No RAM
    case 0x01: mbc_ram_bank_mask = 0x00; break;	// –	Unused 14
    case 0x02: mbc_ram_bank_mask = 0x00; break;	// 8 KiB	1 bank
    case 0x03: mbc_ram_bank_mask = 0x03; break;	// 32 KiB	4 banks of 8 KiB each
    case 0x04: mbc_ram_bank_mask = 0x0F; break;	// 128 KiB	16 banks of 8 KiB each
    case 0x05: mbc_ram_bank_mask = 0x07; break;	// 64 KiB	8 banks of 8 KiB each
    default: mbc_ram_bank_mask = 0x0F; break;
    }
    mbc_flags = 0;
    switch(header_info[0x47]) {
    case 0x00: mbc_type = MBC_Type::Unknown; mbc_rom_bank_mask = 0x01; break; //ROM ONLY
    case 0x01: mbc_type = MBC_Type::MBC1; break; //MBC1
    case 0x02: mbc_type = MBC_Type::MBC1; mbc_flags = MBC_FLAG_RAM; break; //MBC1+RAM
    case 0x03: mbc_type = MBC_Type::MBC1; mbc_flags = MBC_FLAG_BATTERY; break; //MBC1+RAM+BATTERY
    case 0x05: mbc_type = MBC_Type::MBC2; break; //MBC2
    case 0x06: mbc_type = MBC_Type::MBC2; mbc_flags = MBC_FLAG_BATTERY; break; //MBC2+BATTERY
    case 0x08: mbc_type = MBC_Type::Unknown; mbc_flags = MBC_FLAG_RAM; break; //ROM+RAM 11
    case 0x09: mbc_type = MBC_Type::Unknown; mbc_flags = MBC_FLAG_BATTERY; break; //ROM+RAM+BATTERY 11
    case 0x0B: mbc_type = MBC_Type::Unknown; break; //MMM01
    case 0x0C: mbc_type = MBC_Type::Unknown; mbc_flags = MBC_FLAG_RAM; break; //MMM01+RAM
    case 0x0D: mbc_type = MBC_Type::Unknown; mbc_flags = MBC_FLAG_RAM | MBC_FLAG_BATTERY; break; //MMM01+RAM+BATTERY
    case 0x0F: mbc_type = MBC_Type::MBC3; mbc_flags = MBC_FLAG_TIMER | MBC_FLAG_BATTERY; break; //MBC3+TIMER+BATTERY
    case 0x10: mbc_type = MBC_Type::MBC3; mbc_flags = MBC_FLAG_TIMER | MBC_FLAG_RAM | MBC_FLAG_BATTERY; break; //MBC3+TIMER+RAM+BATTERY 12
    case 0x11: mbc_type = MBC_Type::MBC3; break; //MBC3
    case 0x12: mbc_type = MBC_Type::MBC3; mbc_flags = MBC_FLAG_RAM; break; //MBC3+RAM 12
    case 0x13: mbc_type = MBC_Type::MBC3; mbc_flags = MBC_FLAG_RAM | MBC_FLAG_BATTERY; break; //MBC3+RAM+BATTERY 12
    case 0x19: mbc_type = MBC_Type::MBC5; break; //MBC5
    case 0x1A: mbc_type = MBC_Type::MBC5; mbc_flags = MBC_FLAG_RAM; break; //MBC5+RAM
    case 0x1B: mbc_type = MBC_Type::MBC5; mbc_flags = MBC_FLAG_RAM | MBC_FLAG_BATTERY; break; //MBC5+RAM+BATTERY
    case 0x1C: mbc_type = MBC_Type::MBC5; mbc_flags = MBC_FLAG_RUMBLE; break; //MBC5+RUMBLE
    case 0x1D: mbc_type = MBC_Type::MBC5; mbc_flags = MBC_FLAG_RUMBLE | MBC_FLAG_RAM; break; //MBC5+RUMBLE+RAM
    case 0x1E: mbc_type = MBC_Type::MBC5; mbc_flags = MBC_FLAG_RUMBLE | MBC_FLAG_RAM | MBC_FLAG_BATTERY; break; //MBC5+RUMBLE+RAM+BATTERY
    case 0x20: mbc_type = MBC_Type::Unknown; break; //MBC6
    case 0x22: mbc_type = MBC_Type::Unknown; break; //MBC7+SENSOR+RUMBLE+RAM+BATTERY
    case 0xFC: mbc_type = MBC_Type::Unknown; break; //POCKET CAMERA
    case 0xFD: mbc_type = MBC_Type::Unknown; break; //BANDAI TAMA5
    case 0xFE: mbc_type = MBC_Type::Unknown; break; //HuC3
    case 0xFF: mbc_type = MBC_Type::Unknown; break; //HuC1+RAM+BATTERY
    }

    if (header_info[0x47] == 0x1A && header_info[0x50] == 0xDD && memcmp((char*)&header_info[0x51], "SPIDER", 6) == 0) {
        //Special spider cart override, if we are MBC5
        mbc_type = MBC_Type::Spider;
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
    default:
        printf("?");
        break;
    }
}

const uint8_t loader_gb_data[] = {
    #include "Loader.gb.inc"
};

enum class SavState {
    Idle,
    WaitTillSave,
    PostSaveDelay,
} sav_state = SavState::Idle;
absolute_time_t sav_timer;

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
        switch(sav_state) {
        case SavState::Idle: break;
        case SavState::WaitTillSave:
            if (absolute_time_diff_us(get_absolute_time(), sav_timer) < 0) {
                printf("Saving sram");
                save_sav(mbc_ram_bank_mask + 1);
                sav_state = SavState::PostSaveDelay;
                sav_timer = make_timeout_time_ms(10000);
            }
            break;
        case SavState::PostSaveDelay:
            if (absolute_time_diff_us(get_absolute_time(), sav_timer) < 0) {
                sav_state = SavState::Idle;
            }
            break;
        }
        if (multicore_fifo_rvalid()) {
            auto cmd = sio_hw->fifo_rd;
            if (cmd == 0) {
                // request to save SRAM
                if (sav_state == SavState::Idle) {
                    sav_state = SavState::WaitTillSave;
                    sav_timer = make_timeout_time_ms(100);
                }
                if (sav_state == SavState::PostSaveDelay) {
                    sav_state = SavState::WaitTillSave;
                }                
            } else if (cmd & 0x100) {
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
                    stop_mbc();
                    if (load_rom((const char*)&ram_data[15 * 0x2000])) {
                        ram_data[15 * 0x2000 + 0x1FFF] = 1;
                        start_mbc(false);
                    } else {
                        ram_data[15 * 0x2000 + 0x1FFF] = 0;
                        prepare_mbc();
                        if (mbc_flags & MBC_FLAG_BATTERY) {
                            load_sav();
                        }
                        start_mbc(true);
                    }
                    break;
                case COMMAND_LOAD_FOR_QUICKBOOT:
                    stop_mbc();
                    ram_data[15 * 0x2000 + 0x1FFF] = 0;
                    if (load_rom((const char*)&ram_data[15 * 0x2000])) {
                        ram_data[15 * 0x2000 + 0x1FFF] = 1;
                    }
                    start_mbc(false);
                    break;
                case COMMAND_EXEC_QUICKBOOT:
                    stop_mbc();
                    prepare_mbc();
                    if (mbc_flags & MBC_FLAG_BATTERY) {
                        load_sav();
                    }
                    start_mbc(false);
                    break;
                default:
                    ram_data[15 * 0x2000 + 0x1FFF] = 1; // indicate an error
                    break;
                }
            }
        }
    }
    return 0;
}
