#include "coprocessor.h"
#include "romload.h"
#include "mbc.h"
#include "mbc_prepare.h"
#include "command.h"
#include "p8.h"

#include "fatfs/ff.h"
#include <pico/multicore.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

uint8_t ram_data[16 * 0x2000];

enum class SavState {
    Idle,
    WaitTillSave,
    PostSaveDelay,
} sav_state = SavState::Idle;
absolute_time_t sav_timer;

bool valid_rom_ext(const char* ext)
{
    if (strcasecmp(ext, ".gb") == 0) return true;
    if (strcasecmp(ext, ".gbc") == 0) return true;
    if (strcasecmp(ext, ".png") == 0) {
        if (tolower(ext[-1]) == '8' && tolower(ext[-2]) == 'p' && tolower(ext[-3]) == '.') {
            return true;
        }
    }
    return false;
}

int co_error(int error_nr, const char* fmt, ...)
{
    va_list arg_ptr;

    va_start(arg_ptr, fmt);
    vsnprintf((char*)&ram_data[15 * 0x2000 + 0x1F00], 0xF0, fmt, arg_ptr);
    printf("%s\n", (char*)&ram_data[15 * 0x2000 + 0x1F00]);
    va_end(arg_ptr);

    ram_data[15 * 0x2000 + 0x1FFF] = error_nr; // indicate an error
    return error_nr;
}

void processCoProcessor()
{
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
        } else if (cmd == 1) {
            auto ptr = ram_data + 0x2000 * 0x08;
            *ptr++ = 0; //seconds
            *ptr++ = 0; //minutes
            *ptr++ = 0; //hours
            *ptr++ = 0; //day counter
            *ptr++ = 0; //flags/state
            for(int n=0; n<16-5; n++)
                *ptr++ = 0;
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
                                    if (ext && valid_rom_ext(ext)) {
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
            case COMMAND_P8_CYCLE_60:
                p8_cycle60();
                break;
            case COMMAND_P8_CYCLE_30:
                p8_cycle30();
                break;
            default:
                ram_data[15 * 0x2000 + 0x1FFF] = 1; // indicate an error
                break;
            }
        }
    }
}