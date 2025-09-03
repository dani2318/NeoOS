#include "RangedBlockDevice.hpp"
#include <core/cpp/Algorithm.hpp>

RangedBlockDevice::RangedBlockDevice()
{
}

void RangedBlockDevice::Initialize(BlockDevice* device, size_t begin, size_t size){
    this->device = device;
    this->begin = begin;
    this->size = size;
    this->device->Seek(SeekPos::Set, this->begin);
}

size_t RangedBlockDevice::Read(uint8_t* data, size_t size)  {
    if(device == nullptr)
        return 0;
    size = min(size, Size() - Position());
    return device->Read(data, size);
}


size_t RangedBlockDevice::Write(const uint8_t* data, size_t size)  {
    if(device == nullptr)
        return 0;

    size = min(size, Size() - Position());
    return device->Write(data, size);
}

bool RangedBlockDevice::Seek(SeekPos pos, int rel) {
    if(device == nullptr)
        return false;
    
    switch (pos)
    {
    case SeekPos::Set:
        return this->device->Seek(SeekPos::Set, this->begin + rel);
    case SeekPos::Current:
        return this->device->Seek(SeekPos::Current, rel);
    case SeekPos::End:
        return this->device->Seek(SeekPos::End, this->begin + this->size);
    }

    return false;
}

size_t RangedBlockDevice::Size() {
    return this->size;
}

size_t RangedBlockDevice::Position() {
    return this->device->Position() - this->begin;
}
