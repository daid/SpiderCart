#include "filesystem.h"
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
        return p8_load(filename);
    }
    strcpy(sav_filename, filename);
    auto sep = strrchr(sav_filename, '.');
    if (!sep) sep = sav_filename + strlen(sav_filename);
    strcpy(sep, ".sav");

    File f(filename);
    if (!f.isOpen()) return co_error(1, "FS Failure");

    size_t read_size;
    uint32_t addr = 0;
    while((read_size = f.read(ram_data, 2048)) > 0) {
        write_memchip(addr, ram_data, read_size);
        addr += read_size;
    }
    return 0;
}

void load_sav()
{
    if (!sav_filename[0]) return;
    File f(sav_filename);
    if (!f.isOpen()) return;
    f.read(ram_data, sizeof(ram_data));
}

void save_sav(int ram_size)
{
    if (!sav_filename[0]) return;
    File f(sav_filename, true);
    if (!f.isOpen()) return;
    f.write(ram_data, 0x2000 * ram_size);
}