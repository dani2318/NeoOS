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

void RangedBlockDevice::Seek(SeekPos pos, int rel) {
    if(device == nullptr)
        return;
    
    switch (pos)
    {
    case SeekPos::Set:
        this->device->Seek(SeekPos::Set, this->begin + rel);
        break;
    case SeekPos::Current:
        this->device->Seek(SeekPos::Current, rel);
        break;
    case SeekPos::End:
        this->device->Seek(SeekPos::End, this->begin + this->size);
        break;
    default:
        break;
    }
}

size_t RangedBlockDevice::Size() {
    return this->size;
}

size_t RangedBlockDevice::Position() {
    return this->device->Position() - this->begin;
}
