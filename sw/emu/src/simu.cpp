#include "mbc_prepare.h"
#include "memchip.h"
#define DIR FF_DIR
#include "../src/fatfs/ff.h"
#undef DIR
#include <pico/multicore.h>
#include <dirent.h>
#include <chrono>
#include <string>
#include <cstring>

static sio_hw_t sio_hw_impl;
sio_hw_t* sio_hw = &sio_hw_impl;

bool multicore_fifo_rvalid() {
    if (sio_hw->fifo_valid) {
        sio_hw->fifo_valid = false;
        return true;
    }
    return false;
}

absolute_time_t get_absolute_time(void)
{
    auto now = std::chrono::steady_clock::now();
    return now.time_since_epoch().count() / 1000;
}

FRESULT f_mount (FATFS* fs, const char* path, uint8_t opt)
{
    return FR_OK;
}

FRESULT f_opendir (FF_DIR* dp, const char* path)
{
    auto dir_ptr = reinterpret_cast<DIR**>(dp);
    std::string local_path = "./" + std::string(path);
    *dir_ptr = opendir(local_path.c_str());
    return (*dir_ptr != nullptr) ? FR_OK : FR_DISK_ERR;
}

FRESULT f_closedir (FF_DIR* dp)
{
    auto dir_ptr = reinterpret_cast<DIR**>(dp);
    closedir(*dir_ptr);
    *dir_ptr = nullptr;
    return FR_OK;
}

FRESULT f_readdir (FF_DIR* dp, FILINFO* fno)
{
    auto dir_ptr = reinterpret_cast<DIR**>(dp);
    auto dirent = readdir(*dir_ptr);
    memset(fno, 0, sizeof(FILINFO));
    if (dirent) {
        fno->fattrib = 0;
        strcpy(fno->fname, dirent->d_name);
    }
    return FR_OK;
}

FRESULT f_open (FIL* fp, const char* path, uint8_t mode)
{
    auto file_ptr = reinterpret_cast<FILE**>(fp);
    std::string local_path = "./" + std::string(path);
    *file_ptr = fopen(local_path.c_str(), "rb"); //TODO: Mode
    printf("fopen: %s %p\n", local_path.c_str(), *file_ptr);
    if (!*file_ptr) return FR_NO_FILE;
    fp->fptr = 0;
    fseek(*file_ptr, 0, SEEK_END);
    fp->obj.objsize = ftell(*file_ptr);
    fseek(*file_ptr, 0, SEEK_SET);
    return FR_OK;
}

FRESULT f_close (FIL* fp)
{
    auto file_ptr = reinterpret_cast<FILE**>(fp);
    fclose(*file_ptr);
    *file_ptr = nullptr;
    return FR_OK;
}

FRESULT f_read (FIL* fp, void* buff, unsigned int btr, unsigned int* br)
{
    auto file_ptr = reinterpret_cast<FILE**>(fp);
    auto res = fread(buff, 1, btr, *file_ptr);
    fp->fptr += res;
    *br = res;
    return FR_OK;
}

FRESULT f_lseek (FIL* fp, FSIZE_t ofs)
{
    auto file_ptr = reinterpret_cast<FILE**>(fp);
    fseek(*file_ptr, ofs, SEEK_SET);
    return FR_OK;
}

FRESULT f_write (FIL* fp, const void* buff, unsigned int btw, unsigned int* bw)
{
    auto file_ptr = reinterpret_cast<FILE**>(fp);
    auto res = fwrite(buff, 1, btw, *file_ptr);
    fp->fptr += res;
    *bw = res;
    return FR_OK;
}

bool core1_running = true;

MBC_Type mbc_type = MBC_Type::Spider;
uint32_t mbc_flags = MBC_FLAG_RAM;
uint32_t mbc_rom_bank_mask = 0xFF;
uint32_t mbc_ram_bank_mask = 0x0F;

void prepare_mbc()
{
    printf("prepare_mbc\n");
    uint8_t header_info[0x100];
    read_memchip(0x100, header_info, 0x100);
    printf("%s\n", header_info);
}

void start_mbc(bool reset)
{
    core1_running = true;
    printf("start_mbc\n");
    if (reset) {
        printf("wanting to reset, but cannot in emulator...\n");
    }
}

void stop_mbc()
{
    core1_running = false;
    printf("stop_mbc\n");
}