#include "hal.h"
#include <arch/i686/interrupts/gdt.h>
#include <arch/i686/interrupts/idt.h>
#include <arch/i686/interrupts/isr.h>
#include <arch/i686/interrupts/irq.h>

void HAL_Inizialize(){
    i686_GDT_Initialize();
    i686_IDT_Initialize();
    i686_ISR_Initialize();
    i686_IRQ_Initialize();
}