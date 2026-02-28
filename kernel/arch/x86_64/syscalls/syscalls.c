#include "drivers/fbtext.h"
#include <drivers/serial.h>
#include <klib/string.h>
#include <klib/memory.h>
#include <stdint.h>
#include <arch/x86_64/cpu/msr.h>

extern void syscall_handler(void);

#define EFER_MSR         0xC0000080
#define IA32_STAR_MSR    0xC0000081
#define IA32_LSTAR_MSR   0xC0000082
#define IA32_FMASK_MSR   0xC0000084

#define KERNEL_CS 0x08
#define KERNEL_DS 0x10
#define USER_CS   0x1B
#define USER_DS   0x23

void syscalls_init(void)
{
    uint64_t efer = rdmsr(EFER_MSR);
    efer |= (1ULL << 0);
    wrmsr(EFER_MSR, efer);

    wrmsr(IA32_LSTAR_MSR, (uint64_t)syscall_handler);

    uint64_t star = 0;
    star |= ((uint64_t)KERNEL_CS) << 32;
    star |= ((uint64_t)USER_CS) << 48;
    star |= ((uint64_t)USER_CS) << 16;
    wrmsr(IA32_STAR_MSR, star);

    wrmsr(IA32_FMASK_MSR, (1ULL << 9));

    serial_puts("syscalls enabled\n");
}

#define SYS_WRITE 1
#define SYS_EXIT 60

static char buf[64];

typedef struct {
    uint64_t rax, rbx, rbp, rdi, rsi, rdx, rcx, r8, r9, r10, r11, r12, r13, r14, r15;
    uint64_t rip, rflags, rsp;
} syscall_context_t;

uint64_t syscall_common_handler(syscall_context_t *ctx) {
    switch (ctx->rax)
    {
        case SYS_WRITE:
        {
            const char *s = (const char *)ctx->rsi;
            fb_print(s, 0xAAAAAA);
            return 0;
        }

        case SYS_EXIT:
            fb_print("\nUser program exited with code ", 0xAAAAAA);
            u64_to_dec(ctx->rdi, buf);
            fb_print(buf, 0xAAAAAA);
            fb_print("\n", 0xAAAAAA);
            while (1) asm volatile ("hlt");

        default:
            fb_print("unhandled syscall #", 0xAAAAAA);
            u64_to_dec(ctx->rax, buf);
            fb_print(buf, 0xAAAAAA);
            fb_print("\n", 0x000000);
            return -1;
    }
}