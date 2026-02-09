.section .text

.macro PUSH_REGS
    push %rax
    push %rbx
    push %rcx
    push %rdx
    push %rsi
    push %rdi
    push %rbp
    push %r8
    push %r9
    push %r10
    push %r11
    push %r12
    push %r13
    push %r14
    push %r15
.endm

.macro POP_REGS
    pop %r15
    pop %r14
    pop %r13
    pop %r12
    pop %r11
    pop %r10
    pop %r9
    pop %r8
    pop %rbp
    pop %rdi
    pop %rsi
    pop %rdx
    pop %rcx
    pop %rbx
    pop %rax
.endm


.macro EXCEPTION_NOERR num
.global isr\num
isr\num:
    cli
    push $0
    push $\num
    jmp common_noerr
.endm

.macro EXCEPTION_ERR num
.global isr\num
isr\num:
    cli
    push $\num
    jmp common_err
.endm

EXCEPTION_NOERR 0
EXCEPTION_NOERR 6
EXCEPTION_ERR 8
EXCEPTION_ERR 13
EXCEPTION_ERR 14

common_noerr:
    PUSH_REGS

    mov 120(%rsp), %rdi
    mov $0,        %rsi
    mov 136(%rsp), %rdx

    call exception_handler

    POP_REGS
    add $16, %rsp
    iretq


common_err:
    PUSH_REGS

    mov 120(%rsp), %rdi
    mov 128(%rsp), %rsi
    mov 136(%rsp), %rdx

    call exception_handler

    POP_REGS
    add $16, %rsp
    iretq