#pragma once

#include "isr.h"
#include "i8259.h"
#include "pic.h"

typedef void (*IRQHandler) (Registers* regs);

void i686_IRQ_Initialize();
void i686_IRQ_RegisterHandler(int irq, IRQHandler handler);