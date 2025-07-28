[bits 16]

section _TEXT class=CODE

global _x86_TTY_PutChar
_x86_TTY_PutChar:
    push bp
    mov bp, sp

    push bx

    ;   [bp + 0] - old call frame
    ;   [bp + 2] - return address (small memory model => 2 bytes)
    ;   [bp + 4] - first  arg.  (character)
    ;   [bp + 6] - second arg.  (page)

    mov ah, 0Eh
    mov al, [bp + 4]
    mov bh, [bp + 6]

    int 10h

    pop bx

    mov sp, bp
    pop bp
    ret