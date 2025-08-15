#include "BiosDisk.hpp"

EXPORT bool ASMCALL BIOSDiskGetDriveParams(uint8_t drive, uint8_t *driveTypeOut, uint16_t *cylindersOut, uint16_t *sectorsOut, uint16_t *headsOut);
EXPORT bool ASMCALL BIOSDiskReset(uint8_t drive);
EXPORT bool ASMCALL BIOSDiskRead(uint8_t drive, uint16_t cylinder, uint16_t head, uint16_t sector, uint8_t count, uint8_t *dataOut);
EXPORT bool ASMCALL BIOSDiskExtensionPresent(uint8_t drive);
EXPORT bool ASMCALL BIOSDiskExtendedGetDriveParams(uint8_t drive, uint8_t *driveTypeOut, uint16_t *cylindersOut, uint16_t *sectorsOut, uint16_t *headsOut);
EXPORT bool ASMCALL BIOSDiskExtendedRead(uint8_t drive, uint32_t lba, uint8_t count, uint8_t *dataOut);

BIOSDisk::BIOSDisk(uint8_t deviceID)
    : id(deviceID),
      Position(0),
      size(0)
{
}
bool BIOSDisk::Initialize()
{
    haveExtensions = BIOSDiskExtensionPresent(id);

    if (!haveExtensions)
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
}

size_t BIOSDisk::Read(uint8_t *data, size_t size)
{
}

void BIOSDisk::Seek(SeekPos pos, int rel)
{
}

size_t BIOSDisk::Size()
{
}
