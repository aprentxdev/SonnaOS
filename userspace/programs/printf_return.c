#include <printf.h>
#include <syscalls.h>

int main(void)
{
    printf("Hello userspace!\n");
    printf("Dec: %d, Hex: %x, String: %s\n", 42, 0xABCD, "test");
    
    return 42;
}