#pragma once
#include <stddef.h>
#include <stdint.h>
#include "CharacterDevice.hpp"

enum class SeekPos{
    Set,
    Current,
    End
};


class BlockDevice : public CharacterDevice{
    
    virtual size_t Write(const uint8_t* data, size_t size) = 0; 
    virtual size_t Read(uint8_t* data, size_t size) = 0; 
    virtual void Seek(SeekPos pos, int rel) = 0;
    virtual size_t Size() = 0;
};