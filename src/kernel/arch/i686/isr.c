#include "isr.h"
#include "idt.h"
#include "gdt.h"
#include "io.h"
#include <stdio.h>
#include <stddef.h>

ISRHandler g_ISRHandler[256];

static const char* const g_Exceptions[] = {
    "Divide by zero error",
    "Debug",
    "Non-maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack-Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "",
    "x87 Floating-Point Exception",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Control Protection Exception ",
    "",
    "",
    "",
    "",
    "",
    "",
    "Hypervisor Injection Exception",
    "VMM Communication Exception",
    "Security Exception",
    ""
};

void i686_ISR_InitializeGates();

void i686_ISR_Initialize(){
    i686_ISR_InitializeGates();
    for (int i = 0; i < 256; i++){
        i686_IDT_EnableGate(i);
    }
}

void __attribute__((cdecl)) i686_ISR_Handler(Registers* regs){
    if(g_ISRHandler[regs->interrupt] != NULL){
        g_ISRHandler[regs->interrupt](regs);
    }else if(regs->interrupt >= 32){
        printf("Unhandled interrupt %d!\r\n",regs->interrupt);
    }else{
        printf("===   KERNEL PANIC   ===\r\n");
        printf("========================\r\n");
        printf("Unhandled interrupt!\nException: %s (%d)!\r\n",g_Exceptions[regs->interrupt],regs->interrupt);
        printf("========================\r\n");
        printf("ds=%d\r\n",regs->ds);
        printf("edi=%d\r\n",regs->edi);
        printf("esi=%d\r\n",regs->esi);
        printf("ebp=%d\r\n",regs->ebp);
        printf("kern_esp=%d\r\n",regs->kern_esp);
        printf("ebx=%d\r\n",regs->ebx);
        printf("edx=%d\r\n",regs->edx);
        printf("ecx=%d\r\n",regs->ecx);
        printf("eax=%d\r\n",regs->eax);
        printf("interrupt=%d\r\n",regs->interrupt);
        printf("error=%d\r\n",regs->error);
        printf("eip=%d\r\n",regs->eip);
        printf("cs=%d\r\n",regs->cs);
        printf("eflags=%d\r\n",regs->eflags);
        printf("esp=%d\r\n",regs->esp);
        printf("ss=%d\r\n",regs->ss);
        printf("========================\r\n");
        i686_panic();
    }
}

/**
    uint32_t    ds;
    uint32_t    edi, esi, ebp, kern_esp, ebx, edx, ecx, eax;
    uint32_t    interrupt, error;
    uint32_t    eip, cs, eflags, esp, ss;
 */