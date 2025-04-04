bits 16

section _TEXT class=CODE

;
; void _cdecl x86_div64_32(uint64_t dividend, uint32_t divisor, uint64_t* quotientOut, uint32_t* remaiderOut);
;
global _x86_div64_32
_x86_div64_32:
    ; Make new call frame
    push bp             ; Save old call frame
    mov bp, sp          ; Initialize new call frame

    push bx

    ; Divide uppper 32 bits
    mov eax, [bp + 8]   ; eax - dividend
    mov ecx, [bp + 12]  ; ecx - divisor
    xor edx, edx
    div ecx

    ; Store upper 32 bits of quotient
    mov bx, [bp + 16]
    mov [bx + 4], eax

    ; Divide lower 32 bits
    mov eax, [bp + 4]   ; eax - lower 32 bits
                        ; edx - old remainder
    div ecx

    ; Store results
    mov [bx], eax
    mov bx, [bp + 18]
    mov [bx], edx

    pop bx

    ; Restore call frame
    mov sp, bp
    pop bp

    ret

;
; int 10h ah=0Eh
; args: character, page
;
global _x86_Video_WriteCharTeletype
_x86_Video_WriteCharTeletype:
    ; Make new call frame
    push bp             ; Save old call frame
    mov bp, sp          ; Initialize new call frame

    ; Save bx
    push bx

    ; [bp + 0] - old call frame
    ; [bp + 2] - return address (small memory model - 2 bytes)
    ; [bp + 4] - first argument (character)
    ; [bp + 6] - second argument (page)
    ; Note: bytes are converted to words (you can't push a simple byte on the stack)
    mov ah, 0Eh
    mov al, [bp + 4]
    mov bh, [bp + 6]

    int 10h

    ; Restore bx
    pop bx

    ; Restore call frame
    mov sp, bp
    pop bp

    ret
