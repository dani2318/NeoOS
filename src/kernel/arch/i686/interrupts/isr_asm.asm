[bits 32]

%macro  ISR_NOERRORCODE 1
    global i686_ISR%1
    i686_ISR%1:
        push 0
        push %1
        jmp isr_common
%endmacro

%macro  ISR_ERRORCODE 1
    global i686_ISR%1
    i686_ISR%1:
                             ; cpu pushes an error code to the stack
        push %1              ; interrupt number
        jmp isr_common
%endmacro

extern i686_ISR_Handler

%include "isrs_gen.inc"

isr_common:
    pusha               ; pushes all registers

    xor eax, eax
    mov ax, ds
    push eax

    mov ax, 0x10        ; use kernel data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp            ; pass pointer to stack to C

    call i686_ISR_Handler
    add esp, 4

    pop eax             ; restore old segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popa                ; restore what we pushed
    add esp, 8          ; remove error code and interrupt number
    iret                ; will pop: cs, eip, eflags, ss, esp