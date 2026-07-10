#pragma once

#include "../src/fatfs/ff.h"
#include <stdlib.h>

class File
{
public:
    File(const char* filename, bool open_for_write=false);
    ~File();

    bool isOpen();
    size_t read(void* ptr, size_t size);
    size_t write(const void* ptr, size_t size);
    void seek(size_t position);
    size_t tell();
    size_t size();
    bool eof();

private:
    bool file_open = false;
    FIL file;
};

class ReadDir
{
public:
    struct Entry
    {
        char name[FF_LFN_BUF + 1];
        bool directory;
    };

    ReadDir(const char* path);
    ~ReadDir();

    Entry read();
private:
    bool dir_open = false;
    DIR dir;
};