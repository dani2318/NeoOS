global outb
Outb:
    [bits 32]
    mov dx, [esp + 4]
    mov al, [esp + 8]
    out dx, al
    ret

global inb
Inb:
    [bits 32]
    mov dx, [esp + 4]
    xor eax, eax
    in al, dx
    ret

global outl
Outl:
    ; Stack layout (cdecl):
    ; [esp + 4] = port (uint16_t)
    ; [esp + 8] = value (uint32_t)

    mov dx, [esp + 4]    ; porta I/O → DX
    mov eax, [esp + 8]   ; valore 32 bit → EAX
    out dx, eax          ; output long (32 bit)
    ret

global inl
Inl:
    [bits 32]
    mov dx, [esp + 4]
    xor eax, eax
    in eax, dx
    ret

global cli ; Disable Interrupts
Cli:
    cli
    ret

global sti ; Enable Interrupts
Sti:
    sti
    ret

global panic
Panic:
    cli
    hlt
