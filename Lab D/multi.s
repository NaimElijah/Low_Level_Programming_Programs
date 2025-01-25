global _start
global main
extern main

extern printf
extern puts
extern fgets
extern stdin
extern malloc
extern strcmp

BUFFER_SIZE EQU 600

section .bss
    input_buffer resb BUFFER_SIZE
    struct_one resb 1
    struct_one_num resb BUFFER_SIZE
    struct_two resb 1
    struct_two_num resb BUFFER_SIZE

section .data
    STATE dw 0xACE1  ; 16 bit variable
    MASK dw 0xB400   ; for fibonacci 16 bit LFSR mask variable

    x_struct: db 5
    x_num: db 0xaa, 1,2,0x44,0x4f
    y_struct: db 6
    y_num: db 0xaa, 1,2,3,0x44,0x4f
    
    format: db "%04hx",0
    newLine: db 10,0

section .text
_start:
    mov eax, [esp]       ; argc
    lea ebx, [esp+4]     ; argv (pointer to array of arguments)

    push ebx             ; push argv
    push eax             ; push argc
    call main

    mov eax, 1           ; exit
    xor ebx, ebx         ; exit status
    int 0x80

main:
    push ebp   ;  save to stack the ebp of main
    mov ebp, esp
    pushad

    mov eax, [ebp + 8]          ; eax <- argc
    mov ebx, [ebp + 12]          ; ebx <- argv

    cmp eax, 1
    je default_func

    mov edx, [ebx + 4]              ; edx <- argv[1]

    cmp word [edx], "-I"
    je stdin_argument

    cmp word [edx], "-R"
    je PRNG_argument

    popad
    pop ebp
    ret


default_func:                          ; DEFAULT case    <<<==========================
    mov eax, x_struct
    push eax
    call print_multi

    mov ebx, y_struct
    push ebx
    call print_multi
    
    jmp add_AND_print


stdin_argument:                          ; "-I" case    <<<==========================
    mov eax, struct_one
    push eax
    call get_multi
    add esp,4

    mov eax, struct_two
    push eax
    call get_multi
    add esp,4

    mov eax, struct_one
    push eax
    call print_multi

    mov ebx, struct_two
    push ebx
    call print_multi

    jmp add_AND_print


PRNG_argument:                          ; "-R" case    <<<==========================
    mov eax, struct_one
    push eax
    call generate_AND_print

    mov eax, struct_two
    push eax
    call generate_AND_print

    jmp add_AND_print



add_AND_print:                  ;  function for code reusability
    call add_multi
    add esp,8

    push eax
    call print_multi
    add esp,4

    popad
    mov esp, ebp
    pop ebp
    ret         ;  returning to main


; input: pointer to a struct
; inserts into struct a struct of hexa num (up to 256 bytes)
generate_AND_print:                  ;  function for code reusability
    push ebp
    mov ebp,esp
    mov eax, [ebp+8]

    push eax
    call PRmulti
    call print_multi
    add esp,4
    mov esp,ebp
    pop ebp
    ret         ;  returning to main

























PRmulti:
    push ebp
    mov ebp, esp

    ; Generate the random length (8-bit random number)
    xor ebx,ebx
    call rand_num         ; Generate a random number
    mov bl, al            ; Use low byte of random number as length
    test bl, bl           ; Check if length is zero
    jnz valid_len      ; If not zero, continue
len_gen:
    call rand_num         ; Generate another random number
    mov bl, al            ; Use low byte of random number
    test bl, bl           ; Check again
    jz len_gen    ; Repeat if zero

valid_len:
    ; Generate and store multi-precision integer
    mov edi, [ebp + 8]       ; Set result buffer pointer
    mov [edi], bl
    inc edi
bytes_gene:
    call rand_num         ; Generate a random number
    mov [edi], al         ; Store AL in the result buffer
    inc edi
    dec ebx
    jnz bytes_gene  

    mov ebp,esp
    pop ebp
    ret

rand_num:
    ; Load STATE and MASK into registers
    mov ax, [STATE]       ; Load STATE into AX
    mov dx, [MASK]        ; Load MASK into DX

    ; Mask relevant bits
    and ax, dx            ; AX = STATE & MASK

    ; Calculate parity of masked bits
    xor cx, cx            ; Clear CX (parity accumulator)
parity_loop:
    shr ax, 1             ; Shift right AX
    adc cx, 0             ; Add carry flag (parity bit) to CX
    test ax, ax           ; Check if AX is zero
    jnz parity_loop       ; Repeat if not zero
    
    and cx, 1             ; Final parity is in CX (1-bit result)

    ; Shift STATE and update MSB with parity
    mov ax, [STATE]       ; Reload STATE into AX
    shr ax, 1             ; Logical right shift
    shl cx, 15            ; Move parity bit to MSB position
    or ax, cx             ; Set MSB based on parity
    mov [STATE], ax       ; Save updated STATE

    ; Return the updated STATE as the random number
    movzx eax, ax         ; Zero-extend AX into EAX
    ret



































;input: struct_one, struct_two
; output(eax) : struct3 = struct_one+struct_two
add_multi:
    push    ebp
    mov     ebp, esp
    sub     esp, 12               ;local var for new struct pointer

    mov eax, [ebp + 8]      ; = struct_one
    mov ebx, [ebp + 12]      ; = struct_two

    xor ecx, ecx
    xor edx, edx
    mov cl, [eax]           ; cl = size of struct_one
    mov dl, [ebx]           ; dl = size of struct_two
    call MaxMin             ; puts the bigger number/struct, size wise, in eax
addition_setup:
    mov dword [ebp - 8], ecx       ; save the bigger size localy
    mov dword [ebp - 12], edx      ; save the smaller size localy
    mov esi,ecx
    add esi,2
    push edx
    pushad
    push esi
    call malloc
    mov dword [ebp-4], eax
    pop esi
    popad
    pop edx

    dec esi                         ;real size
    mov edi, [ebp - 4]                ; pointer to the struct
    mov ecx, esi
    mov byte [edi], cl             ; put the size field

    ;; EAX - pointer to the larger array (starts from the second byte)
    ;; EBX - pointer to the smaller array (starts from the second byte)
    ;; EDX - size of the smaller array (number of data bytes to sum)
    ;; [EBP - 8] - size of the larger array (number of data bytes to sum)
    ;; EDI - pointer to the result array (starting from the second byte)

    clc   ; Clear carry flag at the start of the addition

    ; Adjust pointers to skip the first byte (length byte) in each array
    inc eax  ; Now EAX points to the second byte of the larger array
    inc ebx  ; Now EBX points to the second byte of the smaller array
    inc edi  ; Now EDI points to the second byte of the result array
    xor ecx, ecx    ;ecx will bee thee last carry

    ; loop to process the bytes from both arrays
addition_loop:
    ; Check if we have finished processing the bytes of the smaller array
    cmp edx, 0           ; Compare the size of the smaller array with 0
    je finish_smaller_array ; If we finished the smaller array, jump to the finish_smaller_array section

    ; Preserve EAX and EBX before modifying AL and BL
    push eax
    push ebx

    ; Load the next byte from the larger array and the smaller array into temporary registers
    mov al, byte [eax]    ; Load byte from larger array into AL
    mov bl, byte [ebx]    ; Load byte from smaller array into BL

    ; Add the two bytes with carry
carry:
    add al, cl            ;adding the last carry
    mov ecx,0
    adc cl, 0             ;if carried,, move it to next iteration
    add al, bl            ; AL = AL + BL (addition)
    adc cl, 0             ; Add the carry to CL (to the next addition)

    ; Store the result in the result array
    mov byte [edi], al    ; Store the result byte in the result array

    ; Restore the values of EAX and EBX
    pop ebx
    pop eax

    ; Increment the pointers to move to the next byte in the arrays
    inc eax
    inc ebx
    inc edi

    ; Decrement the loop counter and continue the loop
    dec edx
    jmp addition_loop

finish_smaller_array:
    ; Process remaining bytes from the larger array (if any)
    ; EDX == 0, but we still have bytes left in the larger array
    mov esi, dword [ebp - 12]
    mov edx, [ebp - 8]
    sub edx, esi                ;get the size of bytes to complete
    
    jz finish_addition        ; If no bytes are left in the larger array, jump to finish_addition
    
process_larger_array:
    ; Preserve EAX before modifying AL
    push eax

    ; Load the next byte from the larger array into AL
    mov al, byte [eax]    ; Load byte from larger array into AL

    ; Add the carry if any
carry1:
    add al, cl             ; Add the last carry to AL
    mov ecx,0              ; from now on no carry
    adc cl,0

    ; Store the result in the result array
    mov byte [edi], al    ; Store the result byte in the result array

    ; Restore EAX after modification
    pop eax

    ; Increment the pointers to move to the next byte in the arrays
    inc eax
    inc edi

    ; Decrement the loop counter for the larger array
    dec edx   ; Decrease the remaining size of the larger array
    cmp edx, 0 ; Check if we have processed all bytes of the larger array
    jz finish_addition
    jmp process_larger_array ; Continue if there are more bytes left in the larger array

MaxMin:   ; according to the assignment, this function is not supposed to use the c calling convention and needs to just put the greater number, size wise, in eax
    cmp cl, dl
    jge no_swap

    xchg eax, ebx
    xchg ecx, edx

    no_swap:
    ret

finish_addition:
    mov byte [edi], cl              ;if there was carry in last addition
    
    mov eax, dword [ebp-4]               ; save the addition result array to return   <<---------------
    mov esp, ebp
    pop ebp
    ret































; input: pointer tto unintialized struct
; inputs from user hexa num and pputs in the struct
get_multi:
    push ebp
    mov ebp,esp

    call get_input_and_its_size          ;cl holds the size
    mov eax, [ebp + 8] ; eax <<- pointer tto the struct

    push eax                 ;pointer to the struct
    push ecx                 ;size of buffer
    call convert_string_to_hexa
    pop ecx
    pop eax

    rcr cl, 1
    adc cl, 0
    mov byte [eax], cl

    mov esp,ebp
    pop ebp
    ret

; input: pointer to empty struct
; inputs from user to input_buffer
;returns input size -> cl
get_input_and_its_size:
    push ebp
    mov ebp, esp

    mov eax, input_buffer
    push dword [stdin]
    push BUFFER_SIZE
    push eax
    call fgets    ; fgets(input_buffer, BUFFER_SIZE, stdin)
    add esp, 12

    mov eax, input_buffer
    xor ecx, ecx
    xor edx, edx
check_size:
    mov dl, [eax]

    cmp dl, 0
    je end_check_size
    cmp dl, 10
    je end_check_size

    inc cl
    inc eax
    jmp check_size
end_check_size:             ;cl <- size
    mov esp,ebp
    pop ebp
    ret

;function. input: size, string. converts into hexa number. -> struct_num
convert_string_to_hexa:
    push ebp
    mov ebp, esp
    
    mov esi, [ebp + 12]                        ; index to struct_num array
    inc esi
    xor edi, edi
    mov ecx, [ebp + 8]                    ;size
    mov ebx, input_buffer                   ;string

    mov edi, ecx

    rcr edi, 1
    jnc even_size
    ; odd size
    xor eax, eax
    mov al, [ebx]
    push eax
    call make_hexa_num
    add esp, 4
    mov byte [esi + edi], al

    inc ebx
    dec cl
even_size:
    dec edi
continue:
    cmp cl, 0
    jle end_convert_string_to_hexa

    ;; high hexa digit
    xor eax, eax
    mov al, [ebx]
    push eax
    call make_hexa_num
    add esp, 4

    mov dl, al
    ; low hexa digit
    xor eax, eax
    mov al, [ebx + 1]
    push eax
    call make_hexa_num
    shl edx, 4
    or edx, eax

    mov byte [esi + edi], dl

    add cl, -2
    add ebx, 2
    dec edi
    jmp continue

end_convert_string_to_hexa:
    mov esp,ebp
    pop ebp
    ret


;; input: single char
;; output: hex value of the char (eax)
make_hexa_num: 
    push ebp
    mov ebp, esp

    mov eax, [ebp + 8]
    cmp eax, '9'
    jle numeric_char   ; if its a number

    ;; if it gets to here then it's an a-f letter
    
    sub eax, 'a'   ;  converting to hexa
    add eax, 10    ;  returning it's value

    mov esp,ebp
    pop ebp
    ret

numeric_char:
    sub eax, '0'
    
    mov esp,ebp
    pop ebp
    ret


























print_multi:
    push ebp
    mov ebp, esp
    push esi
    push edi
    
    mov esi, [ebp + 8]     ; Keep struct pointer in esi
    movzx ecx, byte [esi]  ; Size in ecx
    lea edi, [esi + ecx]   ; End pointer in edi
    
print_loop:
    test ecx, ecx
    jz end_print
    
    push ecx
    movzx eax, byte [edi]  ; Current byte
    push eax
    push format
    call printf
    add esp, 8
    pop ecx

    dec edi     ; onto the next one
    dec ecx     ; onto the next one
    jmp print_loop
end_print:
    mov eax, newLine
    push eax
    call printf
    add esp, 4

    mov esp, ebp
    pop ebp
    ret



