#pragma once
#include <dev/BlockDevice.hpp>


namespace arch{
    namespace i686{
        class VGATextDevice : public BlockDevice {
            public:
                virtual size_t Write(const uint8_t* data, size_t size); 
                virtual size_t Read(uint8_t* data, size_t size); 
        };
    }
}