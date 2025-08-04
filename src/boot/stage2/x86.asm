[bits 16]

section _TEXT class=CODE

global __U4D
__U4D:
    shl edx, 16
    mov dx, ax
    mov eax, edx
    xor edx, edx

    shl ecx, 16
    mov cx, bx

    div ecx

    mov ebx, edx
    mov ecx, edx
    shr ecx, 16

    mov edx, eax
    shr edx, 16
    ret

global __U4M
__U4M:
    shl edx, 16
    mov dx, ax
    mov eax, edx

    shl ecx, 16
    mov cx, bx

    mul ecx
    mov edx, eax
    shr edx, 16

    ret

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

; bool _cdecl x86_Disk_Reset(uint8_t drive);

global _x86_Disk_Reset
_x86_Disk_Reset:
    push bp
    mov bp, sp

    mov ah, 0
    mov dl, [bp + 4]
    stc
    int 13h

    mov ax, 1
    sbb ax, 0

    mov sp, bp
    pop bp
    ret


; bool _cdecl x86_Disk_Read(uint8_t drive, uint16_t cylinder, uint16_t head, uint16_t sector, uint8_t count, uint8_t far* dataOut);

global _x86_Disk_Read
_x86_Disk_Read:
    push bp
    mov bp, sp
    
    push bx
    push es

    mov dl, [bp + 4]

    mov ch, [bp + 6]
    mov cl, [bp + 7]
    shl cl, 6

    mov al, [bp + 8]
    and al, 3Fh
    or cl, al
    
    mov dh, [bp + 10]

    mov al, [bp + 12]

    mov bx, [bp + 16]
    mov es, bx
    mov bx, [bp + 14]

    mov ah, 02h
    stc
    int 13h

    mov ax, 1
    sbb ax, 0

    pop es
    pop bx

    mov sp, bp
    pop bp
    ret

; bool _cdecl x86_Disk_GetDriveParams(uint8_t drive, uint8_t* driveTypeOut, uint16_t* cylindersOut, uint16_t* sectorsOut, uint16_t* headsOut);

global _x86_Disk_GetDriveParams
_x86_Disk_GetDriveParams:
    push bp
    mov bp, sp

    push es
    push bx
    push si
    push di

    mov dl, [bp + 4]
    mov ah, 08h
    mov di, 0
    mov es, di
    stc
    int 13h

    mov ax, 1
    sbb ax, 0

    mov si, [bp + 6]
    mov [si], bl

    mov bl, ch
    mov bh, cl
    shr bh, 6
    mov si, [bp + 8]
    mov [si], bx

    xor ch, ch
    and cl, 3Fh
    mov si, [bp + 10]
    mov [si], cx

    mov cl, dh
    mov si, [bp + 12]
    mov [si], cx

    pop di
    pop si
    pop bx
    pop es

    mov sp, bp
    pop bp
    ret