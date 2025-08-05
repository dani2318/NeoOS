#include "hal.h"
#include <arch/i686/gdt.h>
#include <arch/i686/idt.h>

void HAL_Inizialize(){
    i686_GDT_Initialize();
    i686_IDT_Initialize();
}