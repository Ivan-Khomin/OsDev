bits 16

section _ENTRY class=CODE

extern _start
global entry

entry:
    cli
    mov ax, ds
    mov ss, ax
    mov sp, 0
    mov bp, sp
    sti

    ; Expect boot device in dl, send it as argument to cstart function
    xor dh, dh
    push dx
    call _start

    cli
    hlt