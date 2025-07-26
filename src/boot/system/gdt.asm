gdt_start:
  dd 0x0
  dd 0x0

  ; Code segment descriptor
  dw 0xFFFF     ; Limit
  dw 0x0000     ; Base
  db 0x00       ; Base
  db 10011010b  ; Access
  db 11001111b  ; Flags
  db 0x00       ; Base

  ; Data segment descriptor
  dw 0xFFFF     ; Limit
  dw 0x0000     ; Base
  db 0x00       ; Base
  db 10010010b  ; Access
  db 11001111b  ; Flags
  db 0x00       ; Base

gdt_end:

gdt_descriptor:
  dw gdt_end - gdt_start - 1
  dd gdt_start