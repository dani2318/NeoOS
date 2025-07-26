[ORG 0x7C00]
[bits 16]

%define ENDL 0x0D, 0x0A

CODE_OFFSET equ 0x8
DATA_OFFSET equ 0x10

start:

    ; Print boot message.
    mov si, msg_boot_msg        ; Moving si to the value of 'msg_boot_msg'
    call puts                   ; Calls print function.

    ; Setup stack
    cli
    mov ax, 0x00
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti

    ; Load PM

    cli
    lgdt[gdt_descriptor]
    mov eax, cr0
    or  al, 1
    mov cr0, eax
    jmp CODE_OFFSET:PModeMain

    ; Disable interrupts and halt
    cli
    hlt

[bits 32]
PModeMain:
    mov ax, DATA_OFFSET
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov ss, ax
    mov gs, ax
    mov ebp, 0x7C00
    mov esp, ebp

    in al, 0x92
    or al, 2
    out 0x92, al

    jmp $

%include "src/boot/utils/print.asm"
%include "src/boot/string.asm"
%include "src/boot/system/gdt.asm"


times 510-($-$$) db 0
dw 0AA55h