#pragma once
#include "x86.h"

void putc(char c);
void puts(const char* str);
void _cdecl printf(const char* fmt, ...);
int* printf_number(int* argp, int lenght, bool sign, int radix);
