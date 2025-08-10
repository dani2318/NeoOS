#include <dev/TextDevice.hpp>

TextDevice::TextDevice(CharacterDevice* dev)
    : m_dev(dev){
    
}

const char TextDevice::g_HexChars[] = "0123456789abcdef";

bool TextDevice::Write(char c){
   return m_dev->Write(reinterpret_cast<const uint8_t*>(&c),sizeof(c)) == sizeof(c);
}

bool TextDevice::Write(const char* str){
    bool ok = true;
    while(*str)
    {
        ok = ok && Write(*str);
        str++;
    }
    return ok;
}

bool TextDevice::VFormat(const char* fmt, va_list args){
    return false;
}

bool TextDevice::Format(const char* fmt, ...){
    return false;
}

bool TextDevice::FormatBuffer(const char* msg, const void* buffer, size_t count){
    return false;
}