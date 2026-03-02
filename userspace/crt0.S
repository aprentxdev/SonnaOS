.section .text._start
.global _start

_start:
    xor %rbp, %rbp
    
    and $-16, %rsp
    call main
    
    mov %eax, %edi
    call _exit



    hlt