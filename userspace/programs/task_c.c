#include <printf.h>
#include <stdint.h>

int main(void)
{
    printf("[TASK C] started\n");

    for (long long i = 0; ; i++)
    {
        if (i % 500000000 == 0 && i != 0)
        {
            printf("Hello from task C!\n");

            if (i / 500000000 == 2)
                return 0;
        }
    }
}