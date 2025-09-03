#pragma once

#include <core/FS/FileEntry.hpp>
#include <core/FS/FATFileSystem.hpp>
#include <core/FS/FAT/FATHeaders.hpp>

class FATFileEntry : public FileEntry{
    public:
        FATFileEntry();
        void Initialize(FATFileSystem* fs, const FATDirectoryEntry& DirectoryEntry);
        virtual const char* Name() override;
        virtual const FileType Type() override;
        virtual File* Open(FileOpenMode mode) override;
    private:
        FATDirectoryEntry DirectoryEntry;
        FATFileSystem* fs;
};