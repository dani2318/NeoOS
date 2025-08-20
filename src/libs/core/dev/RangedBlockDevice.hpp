#pragma once
#include "BlockDevice.hpp"

class RangedBlockDevice : public BlockDevice{
    public:
        RangedBlockDevice();
        void Initialize(BlockDevice* device, size_t begin, size_t size);
        virtual size_t Write(const uint8_t* data, size_t size)  override;
        virtual size_t Read(uint8_t* data, size_t size)  override;
        bool Seek(SeekPos pos, int rel) override;
        virtual size_t Size() override;
        virtual size_t Position() override;
    private:
        BlockDevice* device;
        size_t begin;
        size_t size;
};