#pragma once
#include <dev/BlockDevice.hpp>
#include "File.hpp"
#include "FileEntry.hpp"
#include <stdbool.h>


enum FileOpenMode{
    Read,
    Write,
    Append
};

/**
 * The FileSystem class is an abstract base class that defines the interface
 * for a file system. It includes pure virtual methods for initializing the 
 * file system with a block device, retrieving file entries in a directory, 
 * and opening files with a specified mode.
 */
class FileSystem{
    public:
        virtual bool Initialize(BlockDevice* device) = 0;
        virtual FileEntry* GetNextFileEntry(File* parent, const FileEntry& previous) = 0;
        virtual File* Open(FileEntry* parent, FileOpenMode mode) = 0;
};