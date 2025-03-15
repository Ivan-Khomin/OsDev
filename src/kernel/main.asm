org 0x0
bits 16

%define ENDL 0xD, 0xA

start:
    ; print message
    mov si, msg_hello
    call puts

.halt:
    cli
    hlt

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

msg_hello: db 'Hello world from KERNEL!', ENDL, 0