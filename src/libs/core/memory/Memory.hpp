#pragma once
#include <boot/bootparams.hpp>
#include <stdint.h>
#include <core/Defs.hpp>
#include <core/Debug.hpp>

struct E820MemoryBlock{
    uint64_t Base;
    uint64_t Length;
    uint32_t Type;
    uint32_t ACPI;
};

enum E820MemoryBlockType{
    E820_USABLE           = 1,
    E820_RESERVED         = 2,
    E820_ACPI_RECLAIMABLE = 3,
    E820_ACPI_NVS         = 4,
    E820_BAD_MEMORY       = 5,
};


EXPORT int ASMCALL E820GetNextBlock(E820MemoryBlock* block, uint32_t* continuationId);
void MemoryDetect(MemoryInfo* memoryInfo);