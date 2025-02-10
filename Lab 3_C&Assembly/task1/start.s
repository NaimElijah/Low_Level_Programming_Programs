SYS_WRITE EQU 4
STDOUT EQU 1
SYS_OPEN EQU 5

global _start
global system_call
extern strlen

section .data
    newline: db 10
    char: db 0
    outFile: dd 1
    inFile: dd 0

section .text
_start:
    pop    dword ecx    ; ecx = argc
    mov    esi,esp      ; esi = argv
    ;; lea eax, [esi+4*ecx+4] ; eax = envp = (4*ecx)+esi+4
    mov     eax,ecx     ; put the number of arguments into eax
    shl     eax,2       ; compute the size of argv in bytes
    add     eax,esi     ; add the size to the address of argv
    add     eax,4       ; skip NULL at the end of argv
    push    dword eax   ; char *envp[]
    push    dword esi   ; char* argv[]
    push    dword ecx   ; int argc

    call    main        ; int main( int argc, char *argv[], char *envp[] )

    mov     ebx,eax
    mov     eax,1
    int     0x80
    nop

main:
    push ebp
    mov ebp, esp
    pushad
    mov edi, [ebp+8]    ; argc
    mov esi, [ebp+12]   ; argv
    call next
    call encode
    popad
    pop ebp
    mov eax, 0
    ret


next:
    push edx
    push dword[esi + 4*edx]
    call strlen
    push eax
    call print_args
    mov ecx, dword[esp+4]
    add esp, 8                  ;  going back to the counter in edx
    call inputFile_outputFile
    pop edx           ; getting the counter
    inc edx           ;  incrementing the counter
    cmp edx, edi      ; loop condition
    jnz next
    ret


print_args:
    mov eax, 4
    mov ebx, 2  ; to stderr
    mov ecx, [esp + 8]   ; the word: dword[esi + 4*edx]
    mov edx, [esp + 4]   ; the word's length in bytes
    int 0x80
    cmp eax, 0
    jl exit_fail
    mov eax, 4
    mov ebx, 2
    mov ecx, newline
    mov edx, 1
    int 0x80
    cmp eax, 0
    jl exit_fail
    ret


inputFile_outputFile:
    cmp word[ecx], "-o"
    je open_output_file
    cmp word[ecx], "-i"
    je open_input_file
    ret
    
open_output_file:
    mov eax, 5
    mov ebx, ecx
    add ebx, 2
    mov ecx, 1101o
    mov edx, 644o
    int 0x80
    cmp eax, 0
    jl exit_fail
    mov dword[outFile], eax
    ret

open_input_file:
    mov eax, 5
    mov ebx, ecx
    add ebx, 2
    mov ecx, 0
    int 0x80
    cmp eax, 0
    jl exit_fail
    mov dword[inFile], eax
    ret

encode:

    read_char:
        mov eax, 3
        mov ebx, dword[inFile]
        mov ecx, char
        mov edx, 1
        int 0x80
        cmp eax, 0
        je finish_encoding
        jl exit_fail

    encode_char:
        cmp byte[char], 'A'
        jl print_char
        cmp byte[char], 'z'
        jg print_char
        inc byte[char]

    print_char:
        mov eax, 4
        mov ebx, dword[outFile]
        mov ecx, char
        mov edx, 1
        int 0x80
        cmp eax, 0
        jl exit_fail
        jmp encode
    
    finish_encoding:
        ret
    
exit_fail:
    mov ebx, 1
    mov eax, 1
    int 0x80











system_call:
    push    ebp             ; Save caller state
    mov     ebp, esp
    sub     esp, 4          ; Leave space for local var on stack
    pushad                  ; Save some more caller state

    mov     eax, [ebp+8]    ; Copy function args to registers: leftmost...        
    mov     ebx, [ebp+12]   ; Next argument...
    mov     ecx, [ebp+16]   ; Next argument...
    mov     edx, [ebp+20]   ; Next argument...
    int     0x80            ; Transfer control to operating system
    mov     [ebp-4], eax    ; Save returned value...
    popad                   ; Restore caller state (registers)
    mov     eax, [ebp-4]    ; place returned value where caller can see it
    add     esp, 4          ; Restore caller state
    pop     ebp             ; Restore caller state
    ret                     ; Back to caller



