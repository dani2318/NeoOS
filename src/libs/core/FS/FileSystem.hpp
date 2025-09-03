#pragma once
#include <dev/BlockDevice.hpp>
#include "File.hpp"
#include "FileEntry.hpp"
#include <stdbool.h>

class FileSystem{
    public:
        virtual bool Initialize(BlockDevice* device) = 0;
        virtual File* RootDirectory() = 0;
};