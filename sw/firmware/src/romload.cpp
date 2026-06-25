#include "fatfs/ff.h"
#include "memchip.h"
#include "romload.h"
#include "coprocessor.h"
#include "p8.h"
#include <stdio.h>
#include <cstring>


char sav_filename[256];

void clear_sav_filename()
{
    sav_filename[0] = 0;
}

int load_rom(const char* filename)
{
    if (strlen(filename) > 7 && strcmp(filename + strlen(filename) - 7, ".p8.png") == 0) {
        return load_p8(filename);
    }
    strcpy(sav_filename, filename);
    auto sep = strrchr(sav_filename, '.');
    if (!sep) sep = sav_filename + strlen(sav_filename);
    strcpy(sep, ".sav");

    FATFS fatfs;
    memset(&fatfs, 0, sizeof(fatfs));
    if (f_mount(&fatfs, "", 0) != FR_OK)
        return 1;
    FIL fp;
    if (f_open(&fp, filename, FA_READ) != FR_OK)
        return 2;
    UINT br;
    uint32_t addr = 0;
    while(f_read(&fp, ram_data, 2048, &br) == FR_OK && br > 0) {
        write_memchip(addr, ram_data, br);
        addr += br;
    }
    f_close(&fp);
    f_unmount("");
    return 0;
}

void load_sav()
{
    if (!sav_filename[0]) return;
    FATFS fatfs;
    memset(&fatfs, 0, sizeof(fatfs));
    if (f_mount(&fatfs, "", 0) != FR_OK)
        return;
    FIL fp;
    if (f_open(&fp, sav_filename, FA_READ) != FR_OK)
        return;
    UINT br;
    f_read(&fp, ram_data, sizeof(ram_data), &br);
    f_close(&fp);
    f_unmount("");
}

void save_sav(int ram_size)
{
    FATFS fatfs;
    memset(&fatfs, 0, sizeof(fatfs));
    if (f_mount(&fatfs, "", 0) != FR_OK)
        return;
    FIL fp;
    if (f_open(&fp, sav_filename, FA_WRITE | FA_CREATE_ALWAYS) != FR_OK)
        return;
    UINT br;
    f_write(&fp, ram_data, 0x2000 * ram_size, &br);
    f_close(&fp);
    f_unmount("");
}