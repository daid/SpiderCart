#include "filesystem.h"
#include <string.h>
#include <stdint.h>

static uint32_t fatfs_usage_counter = 0;
static FATFS fatfs;

File::File(const char* filename, bool open_for_write)
{
    fatfs_usage_counter++;
    if (fatfs_usage_counter == 1) {
        if (f_mount(&fatfs, "", 0) != FR_OK)
            return;
    }
    
    if (f_open(&file, filename, open_for_write ? FA_WRITE | FA_CREATE_ALWAYS : FA_READ) != FR_OK) {
        return;
    }
    file_open = true;
}

File::~File()
{
    if (file_open) {
        f_close(&file);
    }
    fatfs_usage_counter--;
    if (fatfs_usage_counter == 0) {
        f_unmount("");
    }
}

bool File::isOpen()
{
    return file_open;
}

size_t File::read(void* ptr, size_t size)
{
    if (!file_open) return 0;

    UINT br;
    if (f_read(&file, ptr, size, &br) != FR_OK)
        return 0;
    return br;
}

size_t File::write(const void* ptr, size_t size)
{
    if (!file_open) return 0;

    UINT br;
    if (f_write(&file, ptr, size, &br) != FR_OK)
        return 0;
    return br;
}

void File::seek(size_t position)
{
    if (!file_open) return;
    f_lseek(&file, position);
}

size_t File::tell()
{
    if (!file_open) return 0;
    return f_tell(&file);
}

bool File::eof()
{
    return f_eof(&file);
}


ReadDir::ReadDir(const char* path)
{
    fatfs_usage_counter++;
    if (fatfs_usage_counter == 1) {
        if (f_mount(&fatfs, "", 0) != FR_OK)
            return;
    }

    if (f_opendir(&dir, path) != FR_OK) {
        return;
    }
    dir_open = true;
}

ReadDir::~ReadDir()
{
    if (dir_open) {
        f_closedir(&dir);
    }
    fatfs_usage_counter--;
    if (fatfs_usage_counter == 0) {
        f_unmount("");
    }
}

ReadDir::Entry ReadDir::read()
{
    Entry res;
    res.name[0] = 0;
    res.directory = false;

    if (!dir_open)
        return res;

    FILINFO fno;
    if (f_readdir(&dir, &fno) == FR_OK) {
        strcpy(res.name, fno.fname);
        res.directory = (fno.fattrib & AM_DIR);
    }
    return res;
}
