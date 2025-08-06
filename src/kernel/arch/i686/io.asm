global i686_outb
i686_outb:
    [bits 32]
    mov dx, [esp + 4]
    mov al, [esp + 8]
    out dx, al
    ret

global i686_inb
i686_inb:
    [bits 32]
    mov dx, [esp + 4]
    xor eax, eax
    in al, dx
    ret

global i686_outl
i686_outl:
    ; Stack layout (cdecl):
    ; [esp + 4] = port (uint16_t)
    ; [esp + 8] = value (uint32_t)

    mov dx, [esp + 4]    ; porta I/O → DX
    mov eax, [esp + 8]   ; valore 32 bit → EAX
    out dx, eax          ; output long (32 bit)
    ret

global i686_inl
i686_inl:
    [bits 32]
    mov dx, [esp + 4]
    xor eax, eax
    in eax, dx
    ret

global i686_cli ; Disable Interrupts
i686_cli:
    cli
    ret

global i686_sti ; Enable Interrupts
i686_sti:
    sti
    ret

global i686_panic
i686_panic:
    cli
    hlt
