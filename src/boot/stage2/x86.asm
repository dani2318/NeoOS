[bits 16]

section _TEXT class=CODE

; void _cdecl x86_div64_32(uint64_t dividend, uint32_t divisor, uint64_t* quotientOut, uint32_t* reminderOut);

global _x86_div64_32
_x86_div64_32:
    push bp
    mov bp, sp

    push bx

    ; Divide upper 32 bits
    mov eax, [bp + 8]                       ; eax = upper 32 bits of dividend
    mov ecx, [bp + 12]                      ; ecx = divisor
    xor edx, edx
    div ecx                                 ; eax - quot, edx - reminder

    ; Store upper 32 bits of the quotient
    mov ebx, [bp + 16]
    mov [bx + 4], eax

    ; Divide lower 32 bits
    mov eax, [bp + 4]                       ; eax = lower 32 bits of dividend
                                            ; edx = old reminder
    div ecx

    ; Store result
    mov [bx], eax
    mov bx, [bp + 18]
    mov [bx], edx
    
    pop bx

    mov sp, bp
    pop bp
    ret

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