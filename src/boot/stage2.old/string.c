#include "string.h"
#include <stdint.h>

const char* strchr(const char* str, char chr){
    if(str == NULL) return NULL;
    while(*str){
        if(*str == chr){
            return str;
        }
        ++str;
    }
    return NULL;
}

char* strcpy(char* dst, const char* src){

    char* orginalDst = dst;

    if (dst == NULL) return NULL;

    if (src == NULL){
        *dst = '\0';
        return dst;
    }
    while(*src){
        *dst = *src;
        ++src;
        ++dst;
    }
    *dst += '\0';
    return orginalDst;
}

int strlen(const char* str){
    unsigned len = 0;
    while(*str){
        ++len;
        ++str;
    }

    return len;
}

int strcmp(const char* a, const char* b){
    if (a == NULL && b == NULL)
        return 0;

    if(a == NULL || b == NULL)
        return -1;

    while(*a && *b && *a == *b){
        ++a;
        ++b;
    }

    return (*a) - (*b);
}



wchar_t* UTF16ToCodepoint(wchar_t* string, int* codepoint){
    int c1 = *string;
    ++string;
    if(c1 >= 0xd800 && c1 < 0xdc00){
        int c2 = *string;
        ++string;
        return (wchar_t*) (((c1 & 0x3ff) << 10) + (c2 & 0x3ff) + 0x10000);
    }
    *codepoint = c1;
    return string;
}

char* CodepointToUTF16(int codepoint, char* stringOut){
    if(codepoint <= 0x7F){
        *stringOut = (char)codepoint;
    }else if (codepoint <= 0x7FF){
        *stringOut++ = 0xC0 | ((codepoint >> 6) & 0x1F);
        *stringOut++ = 0x80 | (codepoint & 0x3F);
    }else if (codepoint <= 0xFFFF){
        *stringOut++ = 0xE0 | ((codepoint >> 12) & 0xF);
        *stringOut++ = 0x80 | ((codepoint >> 6) & 0x3F);
        *stringOut++ = 0x80 | (codepoint & 0x3F);
    }else if (codepoint <= 0x1FFFFF){
        *stringOut++ = 0xF0 | ((codepoint >> 18) & 0x7);
        *stringOut++ = 0x80 | ((codepoint >> 12) & 0x3F);
        *stringOut++ = 0x80 | ((codepoint >> 6) & 0x3F);
        *stringOut++ = 0x80 | (codepoint & 0x3F);
    }

    return stringOut;
}