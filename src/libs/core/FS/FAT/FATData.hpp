#pragma once
#include "FATHeaders.hpp"
#include "FATFile.hpp"
#include <stdint.h>

constexpr int SectorSize      =  512;
constexpr int MaxFileHandles  =  10;
constexpr int FatCacheSize    =  5;        // In sectors
constexpr uint32_t FatLfnLast = 0x40;

struct FATData
{
    union{
        FATBootSector BootSector;
        uint8_t       BootSectorBytes[SectorSize];
    } BS;

    FATFile RootDirectory;
    FATFile OpenedFiles[MaxFileHandles];

    uint8_t  FatCache[FatCacheSize * SectorSize];
    uint32_t FatCachePosition;

    FATLFNBlock LFNBlocks[FatLfnLast];
    int LFNCount;

};