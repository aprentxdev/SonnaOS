#include "syscalls.h"

long syscall1(long n, long arg1)
{
    long ret;
    asm volatile(
        "syscall\n"
        : "=a"(ret)
        : "a"(n), "D"(arg1)
        : "rcx", "r11", "memory"
    );
    return ret;
}

long syscall3(long n, long arg1, long arg2, long arg3)
{
    long ret;
    asm volatile(
        "syscall\n"
        : "=a"(ret)
        : "a"(n), "D"(arg1), "S"(arg2), "d"(arg3)
        : "rcx", "r11", "memory"
    );
    return ret;
}

long write(int fd, const void *buf, unsigned long count)
{
    return syscall3(SYS_WRITE, fd, (long)buf, count);
}

long read(int fd, void *buf, unsigned long count)
{
    return syscall3(SYS_READ, fd, (long)buf, count);
}

void _exit(int status)
{
    syscall1(SYS_EXIT, status);
    __builtin_unreachable();
}