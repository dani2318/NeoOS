#pragma once
#include <stdint>

void putc(char c);
void puts(const char* str);
void printf(const char* fmt, ...);
int* printf_number(int* argp, int lenght, bool sign, int radix);
