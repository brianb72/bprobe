.model LARGE,C
.data
   IntVectSave  dw      0

.486
.code

        PUBLIC  install_int24_hnd
        PUBLIC   remove_int24_hnd
        PUBLIC  my_int24_hnd

;-------------------------
; Actual Handler Code
;-------------------------
my_int24_hnd PROC far
        mov     al, 3   ; Fail current system call
        iret
my_int24_hnd ENDP

;-------------------------------
; Install the Int 24 handler 
;-------------------------------

install_int24_hnd PROC

; Preserve registers
        push es
; Save the old entry
        mov     ax, 0
        mov     es, ax
        mov     ax, es:[24h*4]
        mov     word ptr IntVectSave, ax
        mov     ax, es:[24h*4 + 2]
        mov     word ptr IntVectSave+2, ax
; Patch the table
        mov     word ptr es:[24h*4], offset my_int24_hnd
        mov     word ptr es:[24h*4 + 2], seg my_int24_hnd
        push ds
        mov ax, 0
        mov ds, ax
        pop ds

; Restore and return
        pop es
        mov ax, 1
        ret
install_int24_hnd ENDP


;-------------------------------
; Restore the Int 24 handler 
;-------------------------------

remove_int24_hnd PROC
; Preserve registers
        push    es
; Restore the table
        mov     ax, 0
        mov     es, ax
        mov     ax, word ptr IntVectSave
        mov     es:[24h*4], ax
        mov     ax, word ptr IntVectSave+2
        mov     es:[24h*4 + 2], ax
; Restore and return
        pop es
        mov ax, 1
        ret
remove_int24_hnd ENDP

END



