#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "disk.h"
#include "memory.h"
#include "stdio.h"

typedef struct{
    DISK* disk;
    uint32_t Offset;
    uint32_t Size;
}Partition;

void MBR_DetectPartition(Partition* part, DISK* disk, void* partition);
bool Partition_ReadSectors(Partition* part, uint32_t lba, uint8_t sectors, void * lowerDataOut);