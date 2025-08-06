[bits 32]

;void __attribute__((cdecl)) i686_IDT_Load(IDTDescriptor* idtDescriptor);

global i686_IDT_Load
i686_IDT_Load:
    push ebp
    mov ebp, esp

    mov eax, [ebp + 8]
    lidt [eax]

    mov esp, ebp
    pop ebp
    ret