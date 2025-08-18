#pragma once

enum class FileType{
    File,
    Directory
};

struct FileEntry{
    char Name[256];
    FileType Type;
    uint8_t FSData[256];
};