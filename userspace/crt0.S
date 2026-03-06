.section .text._start
.global _start

_start:
    mov $0x1B, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    
    xor %rbp, %rbp
    
    and $-16, %rsp
    call main
    
    mov %eax, %edi
    call _exit



    hlt