; Program to count non-empty names in a file
; Ignores empty lines

section .data
    filename db "names.txt", 0       ; File containing names
    countMsg db "Number of names: ", 0
    errorMsg db "Error opening file", 10, 0
    
    LF equ 10                        ; Line feed character
    NULL equ 0                       ; End of string
    
    ; System call constants
    SYS_read equ 0
    SYS_write equ 1
    SYS_open equ 2
    SYS_close equ 3
    SYS_exit equ 60
    
    STDOUT equ 1
    O_RDONLY equ 0                   ; Read only flag

section .bss
    fileDesc resq 1                  ; File descriptor
    buffer resb 4096                 ; Buffer for file reading
    nameCount resq 1                 ; Counter for names

section .text
    global _start

_start:
    ; Initialize name counter
    mov qword [nameCount], 0
    
    ; Open the file
    mov rax, SYS_open
    mov rdi, filename
    mov rsi, O_RDONLY
    mov rdx, 0
    syscall
    
    ; Check if file opened successfully
    cmp rax, 0
    jl errorOpeningFile
    
    ; Store file descriptor
    mov [fileDesc], rax
    
    ; Initialize line tracking
    xor r8, r8                      ; r8 = 0 (empty line flag: 0=empty, 1=non-empty)
    
readLoop:
    ; Read from file
    mov rax, SYS_read
    mov rdi, qword [fileDesc]
    mov rsi, buffer
    mov rdx, 4096
    syscall
    
    ; Check if end of file (rax = 0) or error (rax < 0)
    cmp rax, 0
    jle endReadLoop
    
    ; Process buffer to count names (non-empty lines)
    mov rcx, 0                      ; Initialize buffer index
    
countNamesLoop:
    cmp rcx, rax                    ; Check if we've processed all bytes read
    jge readLoopContinue            ; If yes, read more from file
    
    mov bl, byte [buffer+rcx]       ; Get current character
    
    ; Check if current character is a newline
    cmp bl, LF
    jne notNewline
    
    ; Found a newline, check if line had content
    cmp r8, 1                       ; Was the line non-empty?
    jne skipEmptyLine               ; If empty, don't count it
    
    ; Line had content, increment name counter
    inc qword [nameCount]
    
skipEmptyLine:
    ; Reset line content flag for next line
    xor r8, r8                      ; Mark new line as empty
    jmp nextChar
    
notNewline:
    ; Check if character is whitespace (space, tab)
    cmp bl, ' '
    je nextChar
    cmp bl, 9                       ; Tab character
    je nextChar
    
    ; Non-whitespace character found, mark line as non-empty
    mov r8, 1
    
nextChar:
    inc rcx                         ; Move to next character
    jmp countNamesLoop
    
readLoopContinue:
    ; Continue reading from file
    jmp readLoop
    
endReadLoop:
    ; Check if the last line had content but no final newline
    cmp r8, 1
    jne skipLastLine
    inc qword [nameCount]
    
skipLastLine:
    ; Close the file
    mov rax, SYS_close
    mov rdi, qword [fileDesc]
    syscall
    
    ; Print message
    mov rdi, countMsg
    call printString
    
    ; Print the count
    mov rax, qword [nameCount]
    call writeInteger
    
    ; Exit program
    mov rax, SYS_exit
    mov rdi, 0
    syscall
    
errorOpeningFile:
    ; Print error message
    mov rdi, errorMsg
    call printString
    
    ; Exit with error code
    mov rax, SYS_exit
    mov rdi, 1
    syscall
    
; Function to print a null-terminated string
printString:
    push rbx
    mov rbx, rdi
    mov rdx, 0                      ; Counter for string length
    
strCountLoop:
    cmp byte [rbx], NULL
    je strCountDone
    inc rdx
    inc rbx
    jmp strCountLoop
    
strCountDone:
    cmp rdx, 0
    je prtDone
    
    mov rax, SYS_write
    mov rsi, rdi                    ; Address of string
    mov rdi, STDOUT
    syscall
    
prtDone:
    pop rbx
    ret
    
; Function to write an integer to console
writeInteger:
    push rbp
    mov rbp, rsp
    sub rsp, 16                     ; Allocate buffer on stack
    
    mov rcx, 10                     ; Divisor for base 10
    mov rbx, rsp                    ; Point to buffer
    add rbx, 15                     ; Start at end of buffer
    mov byte [rbx], 0               ; Null terminator
    dec rbx
    
    ; Handle zero case
    cmp rax, 0
    jne convertLoop
    mov byte [rbx], '0'
    dec rbx
    jmp printNumber
    
convertLoop:
    cmp rax, 0
    je printNumber
    
    xor rdx, rdx                    ; Clear rdx for division
    div rcx                         ; Divide rax by 10, remainder in rdx
    add dl, '0'                     ; Convert remainder to ASCII
    mov [rbx], dl                   ; Store digit
    dec rbx                         ; Move buffer pointer
    jmp convertLoop
    
printNumber:
    inc rbx                         ; Adjust pointer to first digit
    mov rdi, rbx
    call printString
    
    ; Print newline
    mov rdi, rsp
    mov byte [rdi], LF
    mov byte [rdi+1], NULL
    call printString
    
    mov rsp, rbp
    pop rbp
    ret
