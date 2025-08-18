#pragma once
#include <core/FS/FileEntry.hpp>

class FATFileEntry : public FileEntry
{
    public:
        FATDirectoryEntry directoryEntry;
};
