#define SYS_WRITE  1
#define SYS_EXIT   60

static inline long syscall3(long n, long arg1, long arg2, long arg3)
{
    long ret;
    asm volatile(
        "mov %1, %%rdi\n"
        "mov %2, %%rsi\n"
        "mov %3, %%rdx\n"
        "mov %4, %%rax\n"
        "syscall\n"
        : "=a"(ret)
        : "er"(arg1), "er"(arg2), "er"(arg3), "i"(n)
        : "rcx", "r11", "memory"
    );
    return ret;
}

static inline void syscall1(long n, long arg1)
{
    asm volatile(
        "mov %0, %%rax\n"
        "mov %1, %%rdi\n"
        "syscall\n"
        :
        : "i"(n), "i"(arg1)
        : "rcx", "r11", "memory"
    );
}

void user_main(void)
{
    const char msg[] = "Hello from userspace!\n";

    syscall3(SYS_WRITE, 1, (long)msg, sizeof(msg)-1);

    syscall1(SYS_EXIT, 42);
}