#pragma once
#include <stdint.h>

void __attribute__((cdecl)) i686_outb(uint16_t port, uint8_t value);
uint8_t __attribute__((cdecl)) i686_inb(uint16_t port);

void __attribute__((cdecl))  i686_outl(uint16_t port, uint32_t value);
uint8_t __attribute__((cdecl)) i686_inl(uint16_t port);

void __attribute__((cdecl)) i686_cli();
void __attribute__((cdecl)) i686_sti();
void i686_iowait();
void __attribute__((cdecl)) i686_panic();