#pragma once
#include <stdint.h>

template<typename T>
uint32_t to_SegmentOffset(T addr){

    uint32_t addr32 = reinterpret_cast<uint32_t>(addr);
    uint32_t segment = (addr32 >> 4) & 0xFFFF;
    uint32_t offset =  addr32 & 0xF;
    return (segment << 16) | offset;
}

template<typename T>
T to_Linear(uint32_t address){
    uint32_t offset = (uint32_t) (address) & 0xFFFF;
    uint32_t segment = (uint32_t) (address) >> 16;

    return T(segment * 16 + offset);
}