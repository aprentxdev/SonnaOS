#include <drivers/fbtext.h>
#include <drivers/serial.h>
#include <klib/string.h>
#include <klib/memory.h>
#include <stdint.h>
#include <arch/x86_64/cpu/msr.h>
#include <drivers/keyboard.h>

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

#define SYS_READ  0
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
            uint64_t fd = ctx->rdi;
            const char *buf = (const char *)ctx->rsi;
            uint64_t len = ctx->rdx;
            
            if (fd == 1) {
                for (unsigned long i = 0; i < len; i++) {
                    fb_put_char(buf[i], 0xAAAAAA);
                }
                return len;
            }
            return -1;
        }

        case SYS_READ:
        {
            uint64_t fd = ctx->rdi;
            char *user_buf = (char *)ctx->rsi;
            uint64_t count = ctx->rdx;

            if (fd != 0 || count == 0) {
                return -1;
            }

            static char line_buffer[256];
            static int line_pos = 0;
            
            while (1) {
                while (!keyboard_has_data()) {
                    asm("pause");
                }
                
                char c = keyboard_get_char();
                
                if (c == '\n') {
                    int copy_size = (line_pos < count) ? line_pos : count;
                    for (int i = 0; i < copy_size; i++) {
                        user_buf[i] = line_buffer[i];
                    }
                    
                    line_pos = 0;
                    
                    fb_put_char('\n', 0xFFFFFF);
                    return copy_size;
                }
                else if (c == '\b' || c == 127) {
                    if (line_pos > 0) {
                        line_pos--;
                        fb_put_char('\b', 0xFFFFFF);
                    }
                }
                else if (c >= 32 && c <= 126) {
                    if (line_pos < sizeof(line_buffer) - 1) {
                        line_buffer[line_pos++] = c;
                        fb_put_char(c, 0xFFFFFF);
                    }
                }
            }
        }

        case SYS_EXIT:
        {
            fb_print("\nUser program exited with code ", 0xAAAAAA);
            u64_to_dec(ctx->rdi, buf);
            fb_print(buf, 0xAAAAAA);
            fb_print("\n", 0xAAAAAA);
            while (1) asm volatile ("hlt");
        }

        default:
        {
            fb_print("unhandled syscall #", 0xAAAAAA);
            u64_to_dec(ctx->rax, buf);
            fb_print(buf, 0xAAAAAA);
            fb_print("\n", 0x000000);
            return -1;
        }
    }
}