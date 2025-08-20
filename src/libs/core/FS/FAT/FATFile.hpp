#pragma once
#include <core/FS/FATFileSystem.hpp>
#include <core/FS/File.hpp>
#include <core/Debug.hpp>
#include <stdint.h>
#include <core/FS/FAT/FATHeaders.hpp>
#include <core/FS/FileEntry.hpp>

class FATFile : public File {
    public:
        FATFile();
        bool Open(FATFileSystem* fileSystem, uint32_t firstCluster, uint32_t size);
        bool OpenFat1216RootDirectory(FATFileSystem* fileSystem, uint32_t rootDirectoryLba, uint32_t rootDirectorySize);
        bool isOpened() const {return this->Opened;};
        bool ReadFileEntry(FATDirectoryEntry* dirEntry);
        size_t Write(const uint8_t* data, size_t size); 

        FileEntry GetNextFileEntry(File* parent, const FileEntry& previous) override;

        size_t Read(uint8_t* data, size_t size) override; 
        bool Seek(SeekPos pos, int rel) override;
        size_t Size() override;
        size_t Position() override;
    private:
        uint8_t  Buffer[SectorSize];
        bool     Opened;
        bool     isRootDirectory;
        uint32_t position;
        uint32_t size;
        uint32_t FirstCluster;
        uint32_t CurrentCluster;
        uint32_t CurrentSectorInCluster;
        FATFileSystem* fs;
};