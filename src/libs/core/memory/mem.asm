%include "src/libs/core/memory/ProtectedMode.inc"
;
; int ASMCALL x86_E820GetNextBlock(E820MemoryBlock* block, uint32_t* continuationId);
;
E820Signature   equ 0x534D4150

global E820GetNextBlock
; Define a fixed low-memory scratch area address (linear physical address).
; YOU MUST ENSURE this physical address is identity-mapped and free for use.
; Adjust RealModeScratchAddr if you prefer a different low-memory location.
RealModeScratchAddr   equ 0x00000800    ; 2 KiB (below 1 MiB) — change if needed
RealModeScratchSize   equ 28            ; 20 bytes for entry + 4 for EBX_out + 4 for flags/padding

; Offsets inside scratch:
S_OFF_ENTRY    equ 0                 ; 20-byte E820 entry output at scratch+0
S_OFF_EBX_OUT  equ 20                ; 4 bytes: place where real-mode stub writes BX (low 16) and zero high 16
S_OFF_FLAGS    equ 24                ; 1 byte: flags (CF), 0 = ok, 1 = error

; E820 entry size we request
E820_REQ_SIZE  equ 20

; registers note: this function is cdecl, expect args on stack:
; [esp] return addr, [esp+4] block, [esp+8] continuationId

E820GetNextBlock:
    push    ebp
    mov     ebp, esp
    pushad
    pushfd

    mov     esi, [ebp + 8]    ; ESI = block (caller linear pointer)
    mov     edi, [ebp + 12]   ; EDI = continuationId (caller pointer or 0)

    ; --- Prepare scratch usage ---
    ; Clear the scratch area (20 bytes) to be safe
    mov     eax, RealModeScratchAddr
    mov     ecx, RealModeScratchSize
    mov     edi, eax
    xor     eax, eax
    rep stosb                 ; BUT rep stosb uses EDI as virtual address in current data segment
                              ; so this will *not* write to physical RealModeScratchAddr unless identity-mapped.
    ; NOTE: The above rep stosb assumes identity-mapped memory. If your environment doesn't identity-map low mem,
    ; allocate or map RealModeScratchAddr accordingly.

    ; --- Copy caller's buffer (if non-null) into scratch (so BIOS writes to low memory) ---
    cmp     dword [ebp + 8], 0
    je      .skip_copy_to_scratch
    ; copy min(20, caller-provided) bytes (we assume caller buffer is at least 20 bytes)
    mov     esi, [ebp + 8]    ; source linear
    mov     edi, RealModeScratchAddr
    mov     ecx, E820_REQ_SIZE
    rep movsb                 ; again assumes identity mapping

.skip_copy_to_scratch:

    ; --- Prepare continuation ID for BIOS ---
    ; We'll preserve the caller's full 32-bit continuation in SavedContHigh (high16) and will write low16 after real-mode.
    xor     ebx, ebx
    cmp     edi, 0
    je      .no_cont_from_caller
    mov     ebx, [ebp + 12]   ; ebx = continuationId value (32-bit)
.no_cont_from_caller:

    ; Save the high 16 bits so we can reconstruct a 32-bit value after real-mode call (best-effort)
    mov     edx, ebx
    shr     edx, 16           ; EDX = high 16 bits
    mov     [RealModeScratchAddr + S_OFF_EBX_OUT + 2], dx ; store high16 at scratch+22 (if mapping allows)

    ; store low 16 bits into BX (this will be used by real-mode)
    ; In protected mode BX is already the low 16 of EBX automatically once we set EBX.
    ; So ensure EBX is set
    ; ebx already contains continuation id (maybe zero)
    ; we'll fall through into the real-mode transition with EBX configured.

    ; Save EBX across pushad/pops in macros: pushad already saved registers
    ; We'll now perform the real-mode INT using your macro.

    ; Convert the physical scratch linear address into segment:offset for ES:DI
    ; segment = RealModeScratchAddr >> 4 ; offset = RealModeScratchAddr & 0xF
    mov     eax, RealModeScratchAddr
    mov     ebx, eax
    shr     ebx, 4              ; EBX = segment
    and     eax, 0xF            ; EAX = offset
    mov     ax, ax              ; keep low 16 of EAX in AX
    ; set ES:DI accordingly (we'll use the LinearToSegOffset macro if you prefer):
    ; Use provided macro if it behaves as you expect; otherwise do direct stores:
    mov     es, bx              ; ES = segment (needs appropriate privilege; if assembler rejects, use macro)
    mov     di, ax

    ; Set registers for INT 15h/E820h. We'll set EAX and EDX in protected mode; after switching to real-mode
    ; only AX and DX will be used, BX will be the low 16 bits of EBX we set above.
    mov     eax, 0x0000E820
    mov     edx, 0x534D4150      ; 'SMAP'
    mov     ecx, E820_REQ_SIZE
    ; EBX is already set (contains continuation low16 in BX)

    ; Now switch to real mode and call INT 15h.
    x86_EnterRealMode

    ; ---------- Real mode region begins ----------
    ; At this point we are in 16-bit real mode (ES:DI points to RealModeScratchAddr)
    ; Incoming registers:
    ; AX = 0xE820 low 16bits
    ; DX = low 16 of 'SMAP'
    ; CX = 20
    ; BX = low 16 of continuation value (best-effort)
    ; ES:DI -> scratch offset

    ; We'll call INT 15h. BIOS will write up to 20 bytes at ES:DI.
    int     0x15

    ; After return, BIOS updates BX (low 16 bits) with continuation; CF is set on error.
    ; Save BX into scratch[S_OFF_EBX_OUT] (low word)
    mov     [di + S_OFF_EBX_OUT], bx   ; store low 16 bits at scratch+20 (ES segment used)
    ; store flags: pushf/pop ax, store AL bit0 to scratch+24
    pushf
    pop     ax
    mov     [di + S_OFF_FLAGS], al     ; low byte of flags: CF is bit0

    ; ---------- Real mode region ends; jump back to protected mode ----------
    x86_EnterProtectedMode

    ; Now in protected mode again. ES:DI used earlier may not be valid now; instead access physical scratch via its linear address.
    ; Read flags
    mov     al, [RealModeScratchAddr + S_OFF_FLAGS]
    test    al, 1
    jnz     .bios_error

    ; Read low 16 bits returned by real-mode into BX_low
    movzx   eax, word [RealModeScratchAddr + S_OFF_EBX_OUT]   ; EAX = low 16 returned
    mov     ebx, eax
    ; Read previously saved high16 (stored earlier)
    movzx   edx, word [RealModeScratchAddr + S_OFF_EBX_OUT + 2] ; we stored saved high16 there earlier
    shl     edx, 16
    or      ebx, edx               ; EBX = reconstructed 32-bit continuation (best-effort)

    ; Copy the 20-byte entry from scratch back to caller's block (if non-null)
    cmp     dword [ebp + 8], 0
    je      .no_copy_back
    mov     esi, RealModeScratchAddr
    mov     edi, [ebp + 8]     ; caller buffer
    mov     ecx, E820_REQ_SIZE
    rep movsb                  ; assumes identity mapping for scratch and caller buffer accessible

.no_copy_back:

    ; If caller provided continuation pointer, write reconstructed EBX
    mov   edi, [ebp + 12]   ; edi = pointer
    test  edi, edi
    je    .no_write_cont
    mov   [edi], eax


.no_write_cont:
    ; Success
    mov     eax, 0
    jmp     .cleanup

.bios_error:
    mov     eax, -1

.cleanup:
    popfd
    popad
    pop     ebp
    ret