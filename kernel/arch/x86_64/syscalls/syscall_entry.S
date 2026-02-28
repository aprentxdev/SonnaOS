.section .text
.global syscall_handler
.type syscall_handler, @function
.align 16

syscall_handler:
    swapgs

    mov     %rsp, %r12
    mov     tss+4(%rip), %rsp

    push    %r15
    push    %r14
    push    %r13
    push    %r12
    push    %r11
    push    %r10
    push    %r9
    push    %r8
    push    %rcx
    push    %rdx
    push    %rsi
    push    %rdi
    push    %rbp
    push    %rbx
    push    %rax

    mov     %rsp, %rdi
    call    syscall_common_handler

    pop     %rax
    pop     %rbx
    pop     %rbp
    pop     %rdi
    pop     %rsi
    pop     %rdx
    pop     %rcx
    pop     %r8
    pop     %r9
    pop     %r10
    pop     %r11
    pop     %r12
    pop     %r13
    pop     %r14
    pop     %r15

    mov     %r12, %rsp

    swapgs
    sysretq