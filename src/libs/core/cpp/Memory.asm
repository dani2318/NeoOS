
; EXPORT void* ASMCALL memcpy(void* dest, void* src, size_t count);

global memcpy
memcpy:
    push ebp
    mov ebp, esp

    mov edi, [ebp + 8]      ; Destination
    mov esi, [ebp + 12]     ; Source
    mov ecx, [ebp + 16]     ; Count

    rep movsb

    mov esp, ebp
    pop ebp
    ret