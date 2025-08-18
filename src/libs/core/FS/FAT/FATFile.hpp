#pragma once
#include <core/FS/File.hpp>
#include <core/Debug.hpp>
#include <stdint.h>
#include <core/FS/FAT/FATHeaders.hpp>
#include <core/FS/FAT/FATFileEntry.hpp>

class FATFile : public File {
    public:
        FATFile();
        bool Open(FATFileEntry* fileEntry);
        bool isOpened() const {return this->Opened;};
        bool ReadFileEntry(FATDirectoryEntry* dirEntry);

    private:
        uint8_t  Buffer[SectorSize];
        bool     Opened;
        uint32_t Position;
        uint32_t Size;
        uint32_t FirstCluster;
        uint32_t CurrentCluster;
        uint32_t CurrentSectorInCluster;
};