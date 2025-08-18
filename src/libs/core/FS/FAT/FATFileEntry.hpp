#pragma once
#include <FileEntry.hpp>

class FATFileEntry : public FileEntry
{
    public:
        FATDirectoryEntry directoryEntry;
};
