%ifndef I_PROTECTED_MODE_INC__
%define I_PROTECTED_MODE_INC__

%macro x86_EnterRealMode 0
    ; Must be invoked from 32-bit protected mode (CLI should be done by caller).
    ; Clear PE bit in CR0 (use full 32-bit op).
    [bits 32]
    mov     eax, cr0
    and     eax, 0xFFFFFFFE      ; clear PE
    mov     cr0, eax

    ; Far jump to real-mode CS:IP (segment 0)
    jmp     0x0000:.rmode_entry

    ; ---------- real-mode code ----------
    [bits 16]
.rmode_entry:
    cli                         ; keep interrupts disabled
    xor     ax, ax
    mov     ds, ax
    mov     es, ax
    mov     ss, ax
    mov     sp, 0x9000          ; safe real-mode stack (adjust if needed)
    ; NOTE: DO NOT STI here - keep interrupts disabled until we are back in protected mode.
%endmacro

%macro x86_EnterProtectedMode 0
    ; Must be invoked from 16-bit real-mode code (with interrupts disabled).
    ; We will set PE=1 from 32-bit code — so do a far jump into a 32-bit code selector
    ; The caller must ensure GDT with code selector 0x08 (and data selector 0x10) exists.
    [bits 16]
    cli
    ; Far jump into a 32-bit stub (selector 0x08)
    jmp     0x08:.pmode_entry

    ; ---------- protected-mode entry ----------
    [bits 32]
.pmode_entry:
    mov     eax, cr0
    or      eax, 1              ; set PE
    mov     cr0, eax

    ; flush by far jump to 0x08 (same selector) to ensure correct CS state
    jmp     0x08:.pmode_flat

.pmode_flat:
    ; We're now in 32-bit protected mode. Configure segments and stack safely.
    mov     ax, 0x10
    mov     ds, ax
    mov     es, ax
    mov     fs, ax
    mov     gs, ax
    mov     ss, ax
    mov     esp, 0x00100000 - 0x1000   ; example: top of first 1MiB area minus some (adjust as needed)
    sti                         ; enable interrupts once stack & IDT have been established by caller
%endmacro
;    1 - linear address
;    2 - (out) target segment (e.g. es)
;    3 - target 32-bit register to use (e.g. eax)
;    4 - target lower 16-bit half of #3 (e.g. ax)
%macro LinearToSegOffset 4
    mov %3, %1
    shr %3, 4
    mov %2, %4
    mov %3, %1
    and %3, 0xf
%endmacro

%endif