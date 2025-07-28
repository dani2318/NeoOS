org 0x0
bits 16

%define ENDL 0x0D, 0x0A

start:
    mov si, msg_startup
    call puts
    
    cli
    hlt

%include "../boot/drivers/system/print.asm"

msg_startup: db "Entering nkernel mode", ENDL, 0
