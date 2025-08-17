#include "BiosDisk.hpp"
#include "RealMemory.hpp"
#include <core/Assert.hpp>
#include <minmax.hpp>
#include "memory.hpp"

EXPORT bool ASMCALL BIOSDiskGetDriveParams(uint8_t drive,
                                           uint8_t *driveTypeOut,
                                           uint16_t *cylindersOut,
                                           uint16_t *sectorsOut,
                                           uint16_t *headsOut);

EXPORT bool ASMCALL BIOSDiskReset(uint8_t drive);

EXPORT bool ASMCALL BIOSDiskRead(uint8_t drive,
                                 uint16_t cylinder,
                                 uint16_t head,
                                 uint16_t sector,
                                 uint8_t count,
                                 uint8_t *dataOut);

EXPORT bool ASMCALL BIOSDiskExtensionPresent(uint8_t drive);

struct ExtendedDriveParameters
{
    uint16_t ParametersSize;
    uint16_t Flags;
    uint32_t Cylinders;
    uint32_t Heads;
    uint32_t SectorsPerTrack;
    uint64_t Sectors;
    uint16_t BytesPerSectors;
};

EXPORT bool ASMCALL BIOSDiskExtendedGetDriveParams(uint8_t drive,
                                                   ExtendedDriveParameters *params);

struct ExtendedReadParameters
{
    uint8_t ParametersSize;
    uint8_t Reserved;
    uint16_t Count;
    uint32_t TransferBuffer;
    uint64_t LBA;
};

EXPORT bool ASMCALL BIOSDiskExtendedRead(uint8_t drive,
                                         ExtendedReadParameters *params);

BIOSDisk::BIOSDisk(uint8_t deviceID)
    : id(deviceID),
      Position(-1),
      size(0)
{
}
bool BIOSDisk::Initialize()
{
    haveExtensions = BIOSDiskExtensionPresent(id);

    if (haveExtensions)
    {
        ExtendedDriveParameters parameters;
        if (!BIOSDiskExtendedGetDriveParams(id, &parameters))
        {
            return false;
        }

        Assert(parameters.BytesPerSectors == SECTOR_SIZE);

        this->size = SECTOR_SIZE * parameters.Sectors;
        
    }
    else
    {
        uint8_t driveType;

        if (!BIOSDiskGetDriveParams(id, &driveType, &cylinders, &sectors, &heads))
        {
            return false;
        }
    }

    return true;
}

size_t BIOSDisk::Write(const uint8_t *data, size_t size)
{
    return 0;
}

bool BIOSDisk::ReadNextSector(){
    uint16_t cylinder, sector, head;

    //DISK_LBA2CHS(disk, lba, &cylinder, &sector, &head);

    for (int i = 0; i < 3; i++)
    {
        // if (BIOSDiskRead(disk->id, cylinder, sector, head, sectors, buffer))
        //     return true;

        // BIOSDiskReset(this->id);
    }
}


size_t BIOSDisk::Read(uint8_t *data, size_t size)
{   
    uint64_t initialPosition = Position;
    if(Position == -1){
        ReadNextSector();
        Position = 0;
    }
    
    while( size > 0){
        size_t bufferpos = Position % SECTOR_SIZE;
        size_t leftInBuffer = SECTOR_SIZE - bufferpos;
        size_t canRead = min(size, leftInBuffer);
        memcpy(data, buffer + bufferpos, canRead);
        size -= canRead;
        data += canRead;
        Position += canRead;

        if(size > 0){
            ReadNextSector();
        }
    }

    return Position - initialPosition;

}

void BIOSDisk::Seek(SeekPos pos, int rel)
{
}

size_t BIOSDisk::Size()
{
    return this->size;
}
