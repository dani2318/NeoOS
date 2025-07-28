#include "stdio.h"

void putc(char c){
    x86_TTY_PutChar(c, 0);
}

void puts(const char* str){
    while (*str){
        putc(*str);
        str++;
    }
}
