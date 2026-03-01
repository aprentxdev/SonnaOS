#pragma once

#define SYS_WRITE 1
#define SYS_EXIT 60

long syscall0(long n);
long syscall1(long n, long a1);
long syscall2(long n, long a1, long a2);
long syscall3(long n, long a1, long a2, long a3);
long syscall4(long n, long a1, long a2, long a3, long a4);
long syscall5(long n, long a1, long a2, long a3, long a4, long a5);
long syscall6(long n, long a1, long a2, long a3, long a4, long a5, long a6);

long write(int fd, const void *buf, unsigned long count);
void _exit(int status);