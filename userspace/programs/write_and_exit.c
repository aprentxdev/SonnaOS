#include "syscalls.h"

void user_main(void)
{
    const char msg[] = "Hello from userspace!\n";
    write(1, msg, sizeof(msg)-1);
    
    _exit(42);
}