#include "mbc_prepare.h"
#include "memchip.h"
#include <pico/multicore.h>
#include <dirent.h>
#include <chrono>
#include <string>

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

extern "C" {
typedef struct {
	uint32_t fsize;			/* File size (invalid for directory) */
	uint16_t fdate;			/* Date of file modification or directory creation */
	uint16_t ftime;			/* Time of file modification or directory creation */
	uint8_t fattrib;		/* Object attribute */
	char    altname[12 + 1];/* Alternative object name */
	char    fname[255 + 1];	/* Primary object name */
} FILINFO;

int f_mount (void* fs, const char* path, uint8_t opt);
int f_opendir (void* dp, const char* path);
int f_closedir (void* dp);
int f_readdir (void* dp, FILINFO* fno);
int f_open (void* fp, const char* path, uint8_t mode);
int f_close (void* fp);
int f_read (void* fp, void* buff, unsigned int btr, unsigned int* br);
int f_write (void* fp, const void* buff, unsigned int btw, unsigned int* bw);
}

int f_mount (void* fs, const char* path, uint8_t opt)
{
    return 0;
}

int f_opendir (void* dp, const char* path)
{
    auto dir_ptr = reinterpret_cast<DIR**>(dp);
    std::string local_path = "./" + std::string(path);
    *dir_ptr = opendir(local_path.c_str());
    return *dir_ptr == nullptr;
}

int f_closedir (void* dp)
{
    auto dir_ptr = reinterpret_cast<DIR**>(dp);
    closedir(*dir_ptr);
    *dir_ptr = nullptr;
    return 0;
}

int f_readdir (void* dp, FILINFO* fno)
{
    auto dir_ptr = reinterpret_cast<DIR**>(dp);
    auto dirent = readdir(*dir_ptr);
    memset(fno, 0, sizeof(FILINFO));
    if (dirent) {
        strcpy(fno->fname, dirent->d_name);
    }
    return 0;
}

int f_open (void* fp, const char* path, uint8_t mode)
{
    auto file_ptr = reinterpret_cast<FILE**>(fp);
    std::string local_path = "./" + std::string(path);
    *file_ptr = fopen(local_path.c_str(), "rb");
    printf("fopen: %s %p\n", local_path.c_str(), *file_ptr);
    if (!*file_ptr) return 4;
    return 0;
}

int f_close (void* fp)
{
    auto file_ptr = reinterpret_cast<FILE**>(fp);
    fclose(*file_ptr);
    *file_ptr = nullptr;
    return 0;
}

int f_read (void* fp, void* buff, unsigned int btr, unsigned int* br)
{
    auto file_ptr = reinterpret_cast<FILE**>(fp);
    auto res = fread(buff, 1, btr, *file_ptr);
    printf("read: %d %p %zd\n", btr, *file_ptr, res);
    *br = res;
    return 0;
}

int f_write (void* fp, const void* buff, unsigned int btw, unsigned int* bw)
{
    auto file_ptr = reinterpret_cast<FILE**>(fp);
    auto res = fwrite(buff, 1, btw, *file_ptr);
    *bw = res;
    return 0;
}

bool core1_running;

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