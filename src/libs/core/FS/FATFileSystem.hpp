#pragma once
#include "FileSystem.hpp"
#include <core/FS/FAT/FATData.hpp>
#include <core/FS/FAT/FATHeaders.hpp>

#include <stddef.h>

class FATFileEntry;

constexpr int FATReqMemory = 0x10000;
class FATFileSystem : public FileSystem{
    public:
        FATFileSystem();
        bool Initialize(BlockDevice* device) override;
        File* RootDirectory() override;
        bool ReadSector(uint32_t lba, uint8_t* buffer, size_t count=1);
        bool ReadSectorFromCluster(uint32_t cluster, uint32_t sectorOffset, uint8_t* buffer);
        uint8_t GetFatType() const {return this->FatType;};
        FATData& GetFatData() const {return *this->Data;};
        uint32_t GetNextCluster(uint32_t currentCluster);

        FATFile* AllocateFile();
        void ReleaseFile(FATFile* file);
        FATFileEntry* AllocateFileEntry();
        void ReleaseFileEntry(FATFileEntry* file);


    private:
        bool ReadBootSector();
        uint32_t ClusterToLba(uint32_t cluster);
        bool ReadFat(uint32_t lbaOffset);
        void Detect();
        BlockDevice* device;
        FATData*  Data;
        uint32_t  DataSectionLba;
        uint8_t   FatType;
        uint32_t  TotalSectors;
        uint32_t  SectorsPerFat;
};