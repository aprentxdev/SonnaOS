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
    push $0
    push $\num
    jmp common
.endm

.macro EXCEPTION_ERR num
.global isr\num
isr\num:
    push $\num
    jmp common
.endm

EXCEPTION_NOERR 0
EXCEPTION_NOERR 1
EXCEPTION_NOERR 2
EXCEPTION_NOERR 3
EXCEPTION_NOERR 4
EXCEPTION_NOERR 5
EXCEPTION_NOERR 6
EXCEPTION_NOERR 7
EXCEPTION_ERR 8
EXCEPTION_NOERR 9
EXCEPTION_ERR 10
EXCEPTION_ERR 11
EXCEPTION_ERR 12
EXCEPTION_ERR 13
EXCEPTION_ERR 14
EXCEPTION_NOERR 15
EXCEPTION_NOERR 16
EXCEPTION_ERR 17
EXCEPTION_NOERR 18
EXCEPTION_NOERR 19
EXCEPTION_NOERR 20
EXCEPTION_ERR 21
EXCEPTION_NOERR 22
EXCEPTION_NOERR 23
EXCEPTION_NOERR 24
EXCEPTION_NOERR 25
EXCEPTION_NOERR 26
EXCEPTION_NOERR 27
EXCEPTION_NOERR 28
EXCEPTION_NOERR 29
EXCEPTION_NOERR 30
EXCEPTION_NOERR 31

common:
    PUSH_REGS

    mov 120(%rsp), %rdi
    mov 128(%rsp), %rsi
    mov 136(%rsp), %rdx
    mov 144(%rsp), %rcx
    mov 152(%rsp), %r8
    mov 160(%rsp), %r9
    pushq 168(%rsp)

    call exception_handler

    add $8, %rsp

    POP_REGS
    add $16, %rsp
    iretq

.global lapic_timer_isr
lapic_timer_isr:
    PUSH_REGS

    call lapic_timer_handler

    POP_REGS
    iretq

.global lapic_error_isr
lapic_error_isr:
    push $0
    push $0xFE
    jmp common

.global keyboard_isr
keyboard_isr:
    PUSH_REGS
    call keyboard_handler
    POP_REGS
    iretq