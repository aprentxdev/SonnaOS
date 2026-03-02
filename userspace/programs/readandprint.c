#include <printf.h>
#include <syscalls.h>

int main(void) {
    char buf[64];

    printf("Type something: ");

    long n = read(0, buf, sizeof(buf) - 1);

    printf("read returned: %d \n", n);
    printf("You typed: %s \n", buf);
    return 0;
}