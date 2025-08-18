#pragma once
#include <File.hpp>
#include <stdint.h>
#include <FS/FAT/FATHeaders.hpp>
#include <FS/FAT/FATFileEntry.hpp>

class FATFile : public File {
    public:
        FATFile();
        bool Open(FATFileEntry* fileEntry);
        bool isOpened() const {return this->Opened;};
    private:
        uint8_t  Buffer[SectorSize];
        bool     Opened;
        uint32_t Position;
        uint32_t Size;
        uint32_t FirstCluster;
        uint32_t CurrentCluster;
        uint32_t CurrentSectorInCluster;
};