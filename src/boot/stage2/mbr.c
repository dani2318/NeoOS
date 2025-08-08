#include "mbr.h"

typedef struct{
    uint8_t attributes;
    uint8_t chsStart[3];
    uint8_t partitionType;
    uint8_t chsEnd[3];
    uint32_t lbaStart;
    uint32_t size;
} __attribute__((packed)) MBREntry;

void MBR_DetectPartition(Partition* part, DISK* disk, void* partition){
    part->disk = disk;
    if(disk->id < 0x80){
        part->Offset =  0;
        part->Size =  (uint32_t) (disk->cylinders) * (uint32_t) (disk->heads) * (uint32_t) (disk->sectors);
    }else{
        MBREntry* entry = (MBREntry*) segmentoffset_to_linear(partition);
        part->Offset = entry->lbaStart;
        part->Size = entry->size;
    }
}

bool Partition_ReadSectors(Partition* part, uint32_t lba, uint8_t sectors, void * lowerDataOut){
    DISK_ReadSectors(part->disk,lba + part->Offset, sectors, lowerDataOut);
}