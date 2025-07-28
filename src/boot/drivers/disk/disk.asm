;
; Convert LBA address to CHS address
;   Parameters:
;       -   ax: LBA address
;   Returns:
;       -   cx [bits 0-5]:  sector number
;       -   cx [bits 6-15]: cylinders
;       -   dh :            head
lba_to_chs:

    push ax
    push dx

    xor dx, dx                              ; dx = 0
    div word [bdb_sectors_per_track]        ; ax = LBA / bdb_sectors_per_track

    inc dx                                  ; dx = (LBA % bdb_sectors_per_track) + 1 = Sector
    mov cx, dx                              ; cx = sector

    xor dx, dx                              ; dx = 0
    div word [bdb_heads]                    ; ax = (LBA / bdb_sectors_per_track) / heads = Cylinder
                                            ; dx = (LBA / bdb_sectors_per_track) % heads = Head

    mov dh, dl                              ; dh = head
    mov ch, al                              ; hc = cylinder (lower 8 bits)
    shl ah, 6
    or cl, ah                               ; put upper 2 bits of cylinder in cl

    pop ax
    mov dl, al                              ; restore DL
    pop ax
    ret

;
; Read sectors from a disk
;   Parameters:
;       -   ax:     LBA address
;       -   cl:     number of sectors to read (up to 128)
;       -   dl:     drive number
;       -   es:bx:  memory address where to store read data
disk_read:

    push ax
    push bx
    push cx
    push dx
    push di

    push cx                                 ; Save CL (number of sectors to read)
    call lba_to_chs                         ; compute CHS
    pop ax                                  ; AL = number of sector to read

    mov ah, 02h
    mov di, 3                               ; Retry counter

.retry:
    pusha                                   ; Save all register, it's unknown what the BIOS modifies
    stc                                     ; Some BIOS'es do not set the carry flag, so we set it here
    int 13h                                 ; if the carry flag is cleared = success
    jnc .done

    popa 
    call disk_reset

    dec di
    test di, di
    jnz .retry

.fail:
    ; All 3 attempts failed!
    jmp floppy_error

.done:
    popa                                    ; restore registers

    pop di
    pop dx
    pop cx
    pop bx
    pop ax
    ret

;
; Reset disk controller
;   Parameters:
;       -   dl:      drive number
disk_reset:
    pusha
    mov ah, 0
    stc
    int 13h
    jnc floppy_error
    popa
    ret
