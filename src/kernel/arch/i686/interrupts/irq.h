#pragma once

#include <arch/i686/interrupts/isr.h>
#include <arch/i686/pic/i8259.h>
#include <arch/i686/pic/pic.h>

typedef void (*IRQHandler) (Registers* regs);

void i686_IRQ_Initialize();
void i686_IRQ_RegisterHandler(int irq, IRQHandler handler);