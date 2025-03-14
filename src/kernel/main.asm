org 0x7C00
bits 16

%define ENDL 0xD, 0xA

start:
    jmp main

;
; Print function
; Params:
;   ds:si points to string
;
puts:
    ; Save registers
    push si
    push ax

.loop:
    lodsb
    or al, al
    jz .done

    mov ah, 0x0E
    mov bh, 0
    int 0x10    ; call bios interrupt

    jmp .loop

.done:
    pop ax
    pop si
    ret

main:
    ; setup data segments
    mov ax, 0
    mov ds, ax
    mov es, ax

    ; setup stack segments
    mov ss, ax
    mov sp, 0x7C00

    ; print message
    mov si, msg_hello
    call puts

    hlt

.halt:
    jmp .halt

msg_hello: db 'Hello world from kernel!', ENDL, 0

times 510-($-$$) db 0
dw 0AA55h