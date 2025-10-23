[bits 32]
[global idt_flush]

idt_flush:
    mov eax, [esp + 4]   ; idt_ptr parametresi
    lidt [eax]           ; IDT'yi yükle
    ret
