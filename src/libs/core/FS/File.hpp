#pragma once

#include <stddef.h>
#include <stdint.h>
#include <core/dev/BlockDevice.hpp>
#include <core/FS/FileEntry.hpp>

class File : BlockDevice{
    public:
        virtual FileEntry* ReadFileEntry() = 0;

};
