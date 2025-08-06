#include <stdint.h>
#include <hal/hal.h>
#include <arch/i686/io.h>
#include <arch/i686/interrupts/irq.h>
#include <arch/generic/cpu.h>

#include "stdio.h"
#include "memory.h"

void timer(Registers* regs){
}

extern uint8_t __bss_start;
extern uint8_t __end;

void __attribute__((section(".entry"))) start(uint16_t bootDrive){

    memset(&__bss_start, 0, (&__end) - (&__bss_start));

    clrscr();
    printf("Loaded Kernel !!!\r\n");

    HAL_Inizialize();

    printf("Initialized HAL !!!\r\n");

    i686_IRQ_RegisterHandler(0, timer);

    print_cpu_info();


    end:
        for(;;);

}
