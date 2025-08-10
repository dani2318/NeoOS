#pragma once
#include <dev/CharacterDevice.hpp>
#include <cpp/TypeTraits.hpp>
#include <stdarg.h>

enum class FormatState {
    Normal       = 0,
    Length       = 1,
    LengthShort  = 2,
    LengthLong   = 3,
    Spec         = 4,
};

enum class FormatLength {
    Default       = 0,
    ShortShort    = 1,
    Short         = 2,
    Long          = 3,
    LongLong      = 4,
};

class TextDevice{
    public:
        TextDevice(CharacterDevice* dev);
        bool Write(char c);
        bool Write(const char* str);
        bool VFormat(const char* fmt, va_list args);
        bool Format(const char* fmt, ...);
        bool FormatBuffer(const char* msg, const void* buffer, size_t count);

        template<typename TNumber>
        bool Write(TNumber number, int base);

    private:
        CharacterDevice* m_dev;
        static const char g_HexChars[];

};


template<typename TNumber>
bool TextDevice::Write(TNumber number, int base) {

    typename MakeUnsigned<TNumber>::type unsNumber;
    bool ok = true;
    if(IsSigned<TNumber>() && number < 0){
        ok = ok && Write('-');
        unsNumber = -number;
    }else{
        unsNumber = number;
    }

    char buffer[32];
    int pos = 0;

    // convert number to ASCII
    do 
    {
        typename MakeUnsigned<TNumber>::type rem = number % base;
        number /= base;
        buffer[pos++] = g_HexChars[rem];
    } while (number > 0);

    // print number in reverse order
    while (--pos >= 0)
        Write(buffer[pos], base);

    return ok;
}