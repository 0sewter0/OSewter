bits 64

global gdt_load

gdt_load:
    lgdt [rdi]               

    push 0x08                 
    lea rax, [.reload_cs]  
    push rax
    db 0x48, 0xCB

.reload_cs:
    mov ax, 0x10              
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov gs, ax
    ret
