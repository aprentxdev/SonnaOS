#include <printf.h>
#include <stdint.h>

int main(void)
{
    printf("[Task A] started (return 42 after 4 'ping')\n");

    for (long long i = 0; ; i++)
    {
        if (i % 500000000 == 0 && i != 0)
        {
            printf("[Task A] ping %lld!\n", i);

            if (i / 500000000 == 4)
                return 42;
        }
    }
}