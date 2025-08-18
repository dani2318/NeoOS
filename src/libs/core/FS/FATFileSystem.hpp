#pragma once
#include "FileSystem.hpp"
#include <FS/FAT/FATData.hpp>
#include <FS/FAT/FATHeaders.hpp>
#include <FS/FAT/FATFileEntry.hpp>

constexpr int FATReqMemory = 0x10000;

class FATFileSystem : public FileSystem{
    public:
        FATFileSystem(void* fsMemory);
        bool Initialize(BlockDevice* device) override;
        FileEntry* GetNextFileEntry(FileEntry* parent, FileEntry* previous) override;
        File* Open(FileEntry* parent, FileOpenMode mode) override;
    private:
        bool ReadBootSector();
        bool ReadSector(uint32_t lba, uint8_t* buffer);
        uint32_t ClusterToLba(uint32_t cluster);
        void Detect();
        BlockDevice* device;
        FATData*  Data;
        uint32_t  DataSectionLba;
        uint8_t   FatType;
        uint32_t  TotalSectors;
        uint32_t  SectorsPerFat;
};